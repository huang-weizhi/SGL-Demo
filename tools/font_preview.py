# -*- coding: utf-8 -*-
"""
font_preview.py -- 把 Inf_lcd_font.h 里的点阵表打印成字符画，用来校验取模结果。

本工程的 ASCII 字体格式（lcd_show_char / lcd_font_get 使用）：
    - 每个字号一张表：const unsigned char ascii_<高><宽>[][每字符字节数]
    - 95 个字符，顺序 = 0x20(空格) ~ 0x7E(~)，下标 = 字符码 - 0x20
    - 每字符字节数 = ceil(宽 / 8) * 高      （每行像素按整字节补齐）
    - 数据走向：逐行式（从上到下，行内从左到右）
    - 位序：低位在前（bit0 = 最左列），1 = 亮点（阴码）

用法：
    python font_preview.py                      查看所有表的尺寸/占用/字符数
    python font_preview.py 3216 A               打印 32 高 16 宽表的 'A'
    python font_preview.py ascii_3216 0 1 -     也可以用完整表名（字符逐个列出）
    python font_preview.py ascii_4824 0123 --width 24 --height 48
                                                新加的表：名称非 4 位数字时需给出宽高
    python font_preview.py 3216 all             打印全部 95 个字符（很长）

选项：
    --header <路径>   指定头文件（默认 ../project/interface/Inf_lcd_font.h）
    --msb             按高位在前渲染（用于反查取模设置是否选错了顺/逆向）
"""

import os
import re
import sys

DEFAULT_HEADER = os.path.normpath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)), '..', 'project', 'interface', 'Inf_lcd_font.h'))

FIRST_CHAR = 0x20          # 空格
LAST_CHAR = 0x7E           # '~'
FULL_COUNT = LAST_CHAR - FIRST_CHAR + 1        # 95

DECL_RE = re.compile(
    r'const\s+(?:unsigned\s+char|uint8_t|unsigned\s+int|char)\s+(\w+)'
    r'\s*\[\s*(\d*)\s*\]\s*\[\s*(\d+)\s*\]')
BYTE_RE = re.compile(r'0x([0-9A-Fa-f]{2})')


def parse_header(path):
    """返回 [(表名, 每字符字节数, 字符数或None, 字节列表), ...]，顺序与头文件一致。

    支持两种写法（PCtoLCD2002 两种常见导出）：
        const unsigned char ascii_4824[][144]     -> 字符数自动推断
        const uint8_t PCtoLCD2022_BitmapFont[95][144]
    """
    with open(path, 'r', encoding='utf-8', errors='replace') as f:
        text = f.read()

    decls = list(DECL_RE.finditer(text))
    tables = []
    for i, m in enumerate(decls):
        end = decls[i + 1].start() if i + 1 < len(decls) else len(text)
        data = [int(b, 16) for b in BYTE_RE.findall(text[m.end():end])]
        count = int(m.group(2)) if m.group(2) else None
        tables.append((m.group(1), int(m.group(3)), count, data))
    return tables


def info_from_name(name):
    """ascii_3216 -> (宽 16, 高 32)；名称不符规范返回 None。

    要求那 4 位数字前面不是字母数字（避免把 PCtoLCD2022_xxx 里的 2022 当成尺寸）。"""
    m = re.search(r'(?:^|[^0-9A-Za-z])(\d{2})(\d{2})(?![0-9])', name)
    if not m:
        return None
    return int(m.group(2)), int(m.group(1))        # 宽, 高


def render(data, width, height, msb=False):
    per_row = (width + 7) // 8
    grid = [['.' for _ in range(width)] for _ in range(height)]
    for y in range(height):
        for bx in range(per_row):
            byte = data[y * per_row + bx]
            for t in range(8):
                x = bx * 8 + t
                if x >= width:
                    break
                bit = (0x80 >> t) if msb else (0x01 << t)
                if byte & bit:
                    grid[y][x] = '#'
    return '\n'.join(''.join(row) for row in grid)


def scan_backslash_lines(path):
    """返回 [(行号, 行内容), ...]：以反斜杠结尾的行。

    C 里行尾反斜杠 = 续行符。PCtoLCD2002 给反斜杠字符本身生成的注释是
    "//60 \\"，行尾这个反斜杠会把**下一行整行**并进注释吞掉，导致花括号
    配对错乱、编译报 "expected ';' at end of declaration"。取模后务必检查。"""
    hits = []
    with open(path, 'r', encoding='utf-8', errors='replace') as f:
        for i, line in enumerate(f, 1):
            if line.rstrip('\r\n').rstrip().endswith('\\'):
                hits.append((i, line.rstrip()))
    return hits


def main(argv):
    path = DEFAULT_HEADER
    msb = False
    width = height = None
    args = []

    i = 0
    while i < len(argv):
        a = argv[i]
        if a == '--header' and i + 1 < len(argv):
            path = argv[i + 1]
            i += 2
            continue
        if a == '--width' and i + 1 < len(argv):
            width = int(argv[i + 1])
            i += 2
            continue
        if a == '--height' and i + 1 < len(argv):
            height = int(argv[i + 1])
            i += 2
            continue
        if a == '--msb':
            msb = True
            i += 1
            continue
        args.append(a)
        i += 1

    if not os.path.isfile(path):
        print('找不到头文件: %s' % path)
        print('用 --header <路径> 指定 Inf_lcd_font.h')
        return 2

    tables = parse_header(path)
    if not tables:
        print('头文件里没找到 const unsigned char xxx[][n] 形式的点阵表')
        return 2

    if not args:
        print('头文件: %s' % path)
        print('%-14s %-10s %-12s %-8s %-10s %s' %
              ('表名', '宽x高', '字节/字符', '字符数', '总字节', '完整性'))
        for name, per, declared, data in tables:
            size = info_from_name(name)
            if size:
                w, h = size
                note = 'ok' if per == ((w + 7) // 8) * h else '与名称不符!'
                dim = '%dx%d' % (w, h)
            else:
                note = '名称非 4 位数字, 用 --width/--height'
                dim = '?'
            count = len(data) // per if per else 0
            if declared is not None and declared != count:
                note = '声明 %d 个字符, 实际凑出 %d 个!' % (declared, count)
            full = '完整95字符' if count == FULL_COUNT else '只有 %d 个字符' % count
            print('%-14s %-10s %-12d %-8d %-10d %s / %s' %
                  (name, dim, per, count, len(data), full, note))

        bad = scan_backslash_lines(path)
        if bad:
            print('\n!! 危险: 发现 %d 处行尾反斜杠（C 续行符，会把下一行吞进注释）' % len(bad))
            for ln, txt in bad[:10]:
                print('   第 %d 行: ...%s' % (ln, txt.strip()[-40:]))
            print('   PCtoLCD2002 给反斜杠字符生成的注释就是这种，'
                  '把注释末尾的反斜杠删掉即可（例如 //60 backslash）')
        else:
            print('\n行尾反斜杠检查: 通过（没有续行符陷阱）')

        print('\n打印某个字符: python font_preview.py %s A' % tables[0][0])
        return 0

    want = args[0]
    tbl = None
    for name, per, declared, data in tables:
        if name == want or name.lower() == want.lower() or \
                name == 'ascii_' + want or name.endswith('_' + want):
            tbl = (name, per, data)
            break
    if tbl is None:
        print('没有表 %s，现有表: %s' % (want, ', '.join(t[0] for t in tables)))
        return 2

    name, per, data = tbl
    size = info_from_name(name)
    if width is None or height is None:
        if size is None:
            print('表名 %s 读不出宽高，请加 --width/--height' % name)
            return 2
        width, height = size

    per_row = (width + 7) // 8
    if per != per_row * height:
        print('注意: 表 %s 每字符 %d 字节, 而 宽%d 高%d 需要 %d 字节' %
              (name, per, width, height, per_row * height))
        height = per // per_row                     # 以表声明字节数为准反推高度
        print('      改按 宽%d 反推高度 = %d 继续渲染' % (width, height))

    chars = []
    for token in args[1:]:
        if token == 'all':
            chars.extend(chr(c) for c in range(FIRST_CHAR, LAST_CHAR + 1))
        else:
            chars.extend(token)

    if not chars:
        print('%s: %d 字节/字符, 共 %d 个字符 (宽%d 高%d)' %
              (name, per, len(data) // per, width, height))
        print('使用例: python font_preview.py %s A' % name)
        return 0

    print('%s  宽%d 高%d  每字符%d字节  共%d个字符  位序=%s' %
          (name, width, height, per, len(data) // per,
           'MSB高位在前' if msb else 'LSB低位在前'))
    for ch in chars:
        idx = ord(ch) - FIRST_CHAR
        start = idx * per
        if idx < 0 or start + per > len(data):
            print('\n%r 超出这张表的范围（表里只有 %d 个字符）' %
                  (ch, len(data) // per))
            continue
        art = render(data[start:start + per], width, height, msb)
        print('\n%r (下标 %d)  亮点 %d 个' % (ch, idx, art.count('#')))
        print(art)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))