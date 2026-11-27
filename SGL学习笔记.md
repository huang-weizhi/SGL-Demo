# SGL 嵌入式 GUI 移植与调试学习笔记

> 项目：FW-CZ01 温控器 UI 移植（旧裸机直刷框架 → SGL 控件框架）
> 硬件：AT32F415CBT7 + 240×320 LCD + W25Q64 SPI Flash（存图片）
> 工程：WorkBench 生成的 FreeRTOS 工程（`app/project/`），Keil MDK
> 记录时间：2026-11-27
> 目标库：sgl-org/sgl（GitHub），设计器 sgl-designer V1.0.0.7

---

## 一、架构要点（这个项目怎么跑起来的）

### 1.1 FreeRTOS 任务划分

| 任务 | 职责 |
|---|---|
| **output** | `ui_handle()` + `sgl_task_handler()`（渲染）+ `sgl_port_flush_finish()`，2ms 循环 |
| **common** | 按键扫描/事件分发（`input_handle`）、`pc_handle` 串口协议、`flash_save`（内部 Flash 掉电保存，**与 W25Q64 无关**） |
| main（调度器前） | `wk_spi1_init`（= W25Q64 的 SPI1 完整初始化）+ `wk_dma1_channel2/3_init` + `lcd_init` 等 |

### 1.2 切页机制（最终形态）

```
按键任务: KEY_LONG_PRESS → ui_page_toggle_request()   ← 只置 volatile 标志
output:   ui_handle() 读标志 → ui_page_toggle() → ui_page_load(id)
             ├── ui_pages_local[id].init()   生成代码，新建页面控件树
             ├── sgl_obj_remove(new) + parent=self + area=coords   ← 三个缺一不可！
             ├── sgl_screen_load(new)        换 active 指针 + 清脏区
             ├── sgl_obj_free(old)           释放旧页面
             └── on_enter()                  绑定动态内容（Flash 图片等）
```

**铁律：所有触碰 SGL 对象树的操作（建/删/改控件、绑图、切页）只在 output 任务做**。
其他任务只置标志、改数据模型，UI 由 `ui_handle()` 下一轮统一执行——这就是旧框架"标志位 → 刷图"模式在新框架的等价物。

### 1.3 Flash 图片渲染链

```
img_ext0 (img_ext 控件, set_read_ops 绑定读回调)
  → 渲染时逐行调 flash_pic_read(addr, out, len)
    → spiflash_read_async_start/done（SPI1 + DMA1_CH2/3，TX 发 0xFF dummy、RX 收数据）
    → 大端 RGB565 → 字节交换 → SGL framebuffer
图片表：app/ui_flash_img.c  flash_pictures[66]（id = 旧工程图号，57 = 底部装饰栏 240×29）
```

---

## 二、踩坑实录（按时间线，重点看"教训"列）

### Bug 1：切页 0→1→0 正常，第二次 0→1 卡死（`sgl_obj_move_child_pos` 死循环）

**真实根因（最后确认）**：`ui_page_load` 动态创建的页面对象**没有满足 SGL 对 page 的隐含约定**。

SGL 的脏区收集 `sgl_dirty_area_harvest`（sgl_core.c）对每个 dirty 对象做裁剪：

```c
sgl_area_t fill_area = sgl_obj_get_fill_rect(obj->parent);   // = parent.coords ∩ parent.area
sgl_area_clip(&fill_area, &obj->coords, &obj->area);
```

- `sgl_rect_create(NULL)` 会把新页面挂到当前 active 页面下（`sgl_obj_init(NULL): parent = sgl_screen_act()`），而那个页面马上要被 free → **必须摘链并 parent = self**（照抄库内 `sgl_page_create` 的约定）；
- 但 `malloc` 出来的对象 `area = (0,0,0,0)`——parent 指向自己后，fill_rect = 全屏 ∩ (0,0,0,0) = **1 像素** → 脏区只有 1×1 → **渲染只画了左上角 1 个像素，屏幕看起来纹丝不动**。

**修复（三件套）**：

```c
if (new_page->parent != new_page) {
    sgl_obj_remove(new_page);
    new_page->parent = new_page;
    new_page->area = new_page->coords;   // ← 就是这行让一切正常了
}
```

**教训**：
- 库对"页面对象"有隐含约定（parent=self、area 有效），从源码的用法反推约定，别只看 API 签名；
- 悬挂指针**不一定立刻崩**——旧页面 free 后 coords/area 恰好残留全屏值，导致 parent 悬挂的 bug"侥幸能跑"，反而掩盖了真问题；
- **"断点全命中 + 程序不卡死 + 屏幕不变" = 渲染管线没跑，而不是执行卡死**。现象分类决定排查方向。

### Bug 2：SGL 库 img_ext 的 flash_buffer 悬挂指针（库 bug，已确认，待提 PR）

**位置**：`sgl/source/widgets/img_ext/sgl_img_ext.c` 简单绘制路径。

**机理**：每帧渲染后 `sgl_free(pixmap_buf)` 但 `img_ext->flash_buffer` 不清 NULL →
① 同一对象第二次渲染时跳过 malloc，Flash 读回调往**已释放内存**写（UAF）；
② 页面销毁时 `SGL_EVENT_DESTROYED` 分支再次 free → **double free** → mtlsf 空闲链损坏 →
③ 下次从池分配控件时拿到坏块 → 对象链写花 → 各种诡异卡死。

**对照实验（用户亲自复现，三种状态）**：

| 固件 | 结果 |
|---|---|
| 库修复版（buffer 一次分配复用，DESTROYED 释放） | ✅ 正常 |
| 库原版 + app 层每帧清指针规避 | ✅ 正常 |
| 库原版裸奔 | ❌ 0→1→0 后第二次 0→1 死 |

**Patch（一行，上游 HEAD 至今未修，值得提 PR）**：

```diff
             if (img_ext->read != NULL && pixmap_buf != NULL) {
                 sgl_free(pixmap_buf);
+                img_ext->flash_buffer = NULL;
             }
```

安全性已验证：`mtlsf_free(NULL)` 直接返回（mtlsf.c 判空）；DESTROYED 分支本就判 NULL（sgl_img_ext.c:438）。

**顺带发现的第二个库 bug**（PR 里一并提）：旋转/缩放路径分配条件写反——`if (flash_buffer != NULL)` 才 malloc，应为 `== NULL`。

**注意**：触发条件较苛刻——同一 img_ext 对象**渲染第二次**才走悬挂路径。静态页面只渲染一次不会触发；一旦页面里有与图片区域相交的动态更新（选中态高亮等）就会偶发。

### 走过的弯路（同样是学习收获）

1. **跨任务竞态假设**（不成立但修复保留）：最初以为是按键任务 free 对象树与渲染并发踩踏。改成请求标志化虽不是本次根因，但架构上是对的，保留。
2. **spiflash_init 重复初始化**（我的锅）：看到"全工程没人调 spiflash_init"就往两个任务里各加一次，结果两个任务并发重配同一 SPI1/DMA 互相踩。**真相：`wk_spi1_init()` 就是 W25Q64 的 SPI1 初始化，main 里调度器启动前已跑过**。
   → **教训：WorkBench 生成的 `wk_*.c` 在 main 里执行，任务里看到的"没初始化"多半是初始化在别处已完成。改初始化前先全局搜一遍。**
3. **SPI/DMA/中断链路**：逐环节验证过（CPOL/CPHA、dummy 循环、FDT 标志轮询、IRQHandler 为空 weak 符号）都没问题——排除法虽然耗时，但每一步都有据可查，最后才能确定根因唯一。

---

## 三、SGL 关键机制笔记（源码结论）

1. **页面对象约定**：`parent = obj`（指向自己）、`area` 必须有效（= coords）。库自建的 screen 满足约定；**动态创建的页面要自己补齐**。
2. **脏区机制**：`sgl_obj_set_dirty` 只置标志 + update_flag；`sgl_task_handler` 每帧 harvest（遍历 active 树，用 parent 的 fill_rect 裁剪）→ 收集脏区 → 逐区域绘制。**控件不 dirty 就不重绘**——静态页面没有帧渲染。
3. **`sgl_obj_remove` 只摘 sibling 链，不清 parent**——摘链后 parent 仍指旧链。
4. **img_ext 的 read 回调**：`void (*)(size_t addr, uint8_t *out, uint32_t len)`——注意参数顺序（addr 在前），与 `spiflash_read(buf, addr, len)` 相反，需适配层（`flash_pic_read`）。
5. **大端图**：.ebm 资源是大端 RGB565，读出后需字节交换（`flash_pic_read` 里的 swap 循环；颜色若红蓝颠倒说明图实际是小端，删交换即可）。
6. **mtlsf 分配器**：单池静态数组（`CONFIG_SGL_HEAP_MEMORY_SIZE = 6144`，sgl_config.h）；free(NULL) 安全；页面 0+1 峰值约 3KB，做菜单页后建议关注余量。
7. **flash_save ≠ SPI Flash**：`flash_save`/`flash_init` 操作**单片机内部 Flash**（掉电保存参数，ReadBank 直接解引用地址），与 W25Q64 的 SPI1 总线完全无关，不构成并发冲突。

---

## 四、SGL 库版本管理与升级

### 当前状态（2026-11-27）

| 项 | 版本 | 说明 |
|---|---|---|
| 设计器 | V1.0.0.7（2026-07-03） | 仓库唯一 tag = 最新。设计器**不内嵌库源码**，只生成调用 API 的代码 |
| 工程库 | ≈ 上游 main 2026-10-24~10-29 快照（上界 `e9f6d64`） | 无版本宏，用 git blob 哈希逐文件匹配确定 |
| 最新 release | SGL_20261108（`d3769e0`，2026-11-08） | |
| 上游 HEAD | `7dd2f54`（2026-11-26） | |

### 版本鉴定方法（可复用）

工程库没有版本号时，用 blob 哈希精确匹配：

```powershell
git clone https://github.com/sgl-org/sgl $env:TEMP\sgl-upstream
git hash-object 本地文件                      # 算本地 blob
git -C $up rev-parse "<commit>:source/路径"   # 算历史 blob，逐一比对
```

再配合"特征文件存在性"（如 winslate 有/无）收窄快照窗口。**web 页面摘要不可靠（AI 转述会误读代码），逐字节 diff 才是硬证据。**

### 快照 → HEAD 的主要差异（52 提交）

- **修复**：mtlsf 分配器加固 #168（对齐语义，`57a057d`）⭐、sprintf hotfix、fs/fatfs 修复、draw rect 修复、arc workaround、LOG_LEVEL 编译修复
- **新功能**：cursor 支持、icon + explorer 控件、winslate/slate 窗口、switch 动画、flash 字体 cache、fbdev TE 信号（刷屏时序优化 ⭐）
- **破坏性**：`sgl_theme.h` 被删除（升级需迁移）
- **图片 bug**：**上游未修**（HEAD 的 img_ext.c 与本地仅差我们的 7 行 patch）。`957e519 "fix img widget issue"` 修的是 `widgets/img/`，不是 `img_ext`

### 升级策略

- **现在不升**：项目刚稳定，`sgl_core.c` 有 11 个提交变更（cursor/TE/字体），需全量回归；
- **升级时机**：img_ext patch 被 PR 合并后一起升；或需要 TE 信号优化刷屏、mtlsf 加固、新控件时；
- **升级操作**：整目录替换 `sgl/source` → 重新 apply img_ext patch（若无冲突）→ 核对 sgl_config.h（theme 宏迁移）→ 回归：切页 ×N、Flash 图渲染、长按交互。
- 升级时 img_ext.c 无冲突风险（上游没动这个文件，本地 patch 可原样重放）。

---

## 五、调试方法论（这次真正学会的东西）

1. **对照实验 > 推理**：三种固件状态（库修复/规避/裸奔）的对比直接锁定根因，比任何猜测都硬。
2. **断点是"现象分类器"**：断点全命中 + 程序不卡死 → 排除"执行卡死"，锁定"渲染不刷新"；断点不命中 → 问题在更早的传导链。
3. **排除法的价值**：按键状态机、请求标志、对象树、内存池、SPI/DMA、中断逐层验证，看似绕路，实际是把根因的存在空间压缩到唯一。
4. **悬挂指针不一定立刻崩**：残留数据"恰好无害"会让 bug 长期潜伏（parent 悬挂侥幸能跑、flash_buffer 悬挂静态页面不触发）。**别把"能跑"当"正确"。**
5. **先查"别人是不是已经做了"**：WorkBench 的 wk_*.c、main 里的初始化，改之前全局搜。
6. **改第三方库要克制**：先在 app 层规避，确认是库 bug 后用最小 patch 修库并回馈上游（PR）——本次从"改库→还原→app 规避→还原→确认 bug→最小 patch"完整走了一遍正确的流程。
7. **提 PR 前的证据链**：复现步骤 + 根因分析 + 最小 diff + 下游安全性验证（free(NULL) 判空、DESTROYED 判空）。

---

## 六、待办

- [ ] 向 sgl-org/sgl 提交 PR：img_ext flash_buffer 悬挂指针（一行 patch + 旋转路径条件反 + malloc 判空，三个问题可一并写）
- [ ] 关注上游 PR 合并情况，合并后评估升级（mtlsf 加固 + TE 信号值得要）
- [ ] 菜单页选中态实现后，注意 img_ext 重复渲染路径（bug 已 patch，无风险，但留意性能：每帧 malloc/free 480B）
- [ ] GUI 堆余量：`CONFIG_SGL_HEAP_MEMORY_SIZE = 6144`，页面控件增多后关注 `sgl_mm_get_monitor()`
