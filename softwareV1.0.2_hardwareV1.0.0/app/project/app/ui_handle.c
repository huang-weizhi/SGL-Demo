#include "ui_handle.h"
#include "ui_flash_img.h"

/* ------------------------------------------------------------------
 * Menu selection state. The two menu pages form ONE virtual list of
 * 10 items that wraps around (legacy firmware behaviour):
 *   global index 0..4 -> screen1 items (page select..speaker)
 *   global index 5..9 -> screen2 items (version..countdown/return)
 * Moving past either end of a page jumps to the other page.
 * Owned exclusively by the UI (output) task: only ui_handle()/on_enter
 * touch menu_index and the item styles. The input task only raises
 * single-byte request flags (see ui_menu_move_request).
 * ------------------------------------------------------------------ */

/* Menu selection effect switch: 1 = animated sliding highlight frame,
 * 0 = legacy colour-swap selection (menu_apply_style) */
#define MENU_HIGHLIGHT_ANIM 1

/* capsule styles: normal = designer defaults, selected = highlighted */
#define MENU_ITEM_BG_NORMAL sgl_rgb(0x30, 0x30, 0x30)
#define MENU_ITEM_BORDER_NORMAL sgl_rgb(0xF0, 0xF8, 0xFF)
#define MENU_ITEM_BG_SELECTED sgl_rgb(0x55, 0x55, 0x55)
#define MENU_ITEM_BORDER_SELECTED sgl_rgb(0x31, 0x5D, 0xDE)

/* ordered menu items per page. Elements point AT the designer globals
 * (not the objects): the globals are re-assigned every time the page is
 * rebuilt by ui_page_load, so always dereference them at use time. */
static sgl_obj_t **menu_items_s1[] = {
    &rectangle4,
    &rectangle5,
    &rectangle6,
    &rectangle7,
    &rectangle8,
};
static sgl_obj_t **menu_items_s2[] = {
    &rectangle9,
    &rectangle10,
    &rectangle11,
    &rectangle13,
    &rectangle12,

};

#define MENU_PAGE2_START 5u /* first global index of screen2 (version..) */
#define MENU_ITEM_TOTAL 10u /* screen1 (5 items) + screen2 (5 items) */

/* global selection 0..9 across both pages; page-local index = gidx-start */
static uint8_t menu_index = 0;
/* set by menu_step when a move crosses pages: menu_enter() then keeps
 * menu_index instead of resetting to the page's first item */
static bool menu_keep_index = false;

/* ------------------------------------------------------------------
 * The single source of truth for which page is active.
 * NEVER compare sgl_screen_act() against the designer globals
 * (screen0/1/2): pages are malloc'd and freed, so those globals dangle
 * after their page dies, and a new page can reuse a freed block - the
 * comparison then randomly matches the WRONG page. Track the id here
 * instead (owned by the output task, updated in ui_page_load only).
 * Power-on loads screen0 via ui_init() (helper path), matching the
 * static initializer below. */
static ui_scr_id_t cur_page_id = UI_SCREEN0;

/* resolve the item list of the active menu page; NULL on non-menu pages.
 * Returns the item table (sgl_obj_t** elements, i.e. addresses of the
 * designer globals). screen1/screen2 globals dangle when their page is
 * freed, but then the active screen is screen0 and the comparisons below
 * are false anyway. */
static sgl_obj_t ***menu_get_items(uint8_t *count)
{
    if (cur_page_id == UI_SCREEN1)
    {
        *count = 5;
        return menu_items_s1;
    }
    if (cur_page_id == UI_SCREEN2)
    {
        *count = 5;
        return menu_items_s2;
    }
    *count = 0;
    return NULL;
}

static void menu_apply_style(uint8_t gidx, bool selected)
{
    sgl_obj_t ***items;
    uint8_t count;
    uint8_t start;
    sgl_obj_t *cap;

    items = menu_get_items(&count);
    if (items == NULL)
    {
        return;
    }
    start = (cur_page_id == UI_SCREEN2) ? MENU_PAGE2_START : 0;
    if (gidx < start || gidx - start >= count)
    {
        return; /* index belongs to the other menu page */
    }
    cap = *items[gidx - start];
    sgl_rect_set_color(cap, selected ? MENU_ITEM_BG_SELECTED : MENU_ITEM_BG_NORMAL);
    sgl_rect_set_border_color(cap, selected ? MENU_ITEM_BORDER_SELECTED : MENU_ITEM_BORDER_NORMAL);
    sgl_obj_set_dirty(cap); /* redraw only this capsule */
}

/* ------------------------------------------------------------------
 * Selection highlight frame: a border-only rect floating above the
 * menu capsules. Instead of an instant colour swap, the frame SLIDES
 * to the selected item (150 ms ease-out quart) via the SGL animation
 * engine (driven by sgl_task_handler -> sgl_anim_task every frame).
 * The fill alpha is 0 so the capsule stays visible underneath; only
 * the 2 px border (MENU_ITEM_BORDER_SELECTED) marks the selection.
 * Created lazily on the active page; it dies with the page - the
 * slide animation is detached in ui_page_load before the page free. */
#if MENU_HIGHLIGHT_ANIM
static sgl_obj_t *menu_highlight = NULL;

/* ---- animation callbacks (absolute coords + explicit dirty areas) ----
 * The stock sgl_anim_move_obj_* callbacks drive parent-relative setters,
 * which breaks on the page object (its parent is itself after the
 * load-time fix: every frame would add the position onto itself) and
 * leaves motion trails behind the highlight frame (sgl_obj_set_dirty
 * only marks the object, the vacated strip is never repainted). These
 * callbacks move in absolute coords; the highlight one also pushes the
 * vacated strip back into the dirty area. */

static void anim_hl_slide_cb(sgl_anim_t *anim, int32_t value)
{
    sgl_obj_t *obj = (sgl_obj_t *)anim->data;
    sgl_area_t trail = obj->coords;
    const int16_t h = obj->coords.y2 - obj->coords.y1 + 1;

    sgl_obj_set_abs_pos(obj, obj->coords.x1, (int16_t)value);
    trail.y1 = sgl_min(trail.y1, (int16_t)value);
    trail.y2 = sgl_max(trail.y2, (int16_t)(value + h - 1));
    sgl_update_area(&trail); /* repaint the vacated strip: no ghost border */
}

static void menu_highlight_sync(bool animate)
{
    sgl_obj_t ***items;
    uint8_t count;
    uint8_t start;
    sgl_obj_t *cap;
    int16_t tx, ty;

    items = menu_get_items(&count);
    if (items == NULL)
    {
        return;
    }
    start = (cur_page_id == UI_SCREEN2) ? MENU_PAGE2_START : 0;
    if (menu_index < start || menu_index - start >= count)
    {
        return;
    }
    cap = *items[menu_index - start];
    if (cap == NULL)
    {
        return;
    }

    if (menu_highlight == NULL)
    {
        menu_highlight = sgl_rect_create(sgl_screen_act());
        if (menu_highlight == NULL)
        {
            return;
        }
        sgl_obj_set_size(menu_highlight,
                         cap->coords.x2 - cap->coords.x1 + 5,
                         cap->coords.y2 - cap->coords.y1 + 5);
        sgl_rect_set_color(menu_highlight, sgl_rgb(0x01, 0x01, 0x01));
        sgl_rect_set_alpha(menu_highlight, 0);   /* border only, keep capsule visible */
        sgl_rect_set_radius(menu_highlight, 10); /* capsule radius 8 + 2 outset */
        sgl_rect_set_border_width(menu_highlight, 4);
        sgl_rect_set_border_color(menu_highlight, MENU_ITEM_BORDER_SELECTED);
        sgl_rect_set_border_alpha(menu_highlight, 255);
        /* created last on the page -> drawn on top of the capsules */
    }

    /* The frame must adopt the SELECTED item's size every time: the
     * return button (60x30) differs from the capsules (193x32), so a
     * size fixed at creation time is wrong whenever the selection lands
     * on an item of a different shape. Outset 2 px + 4 px border puts
     * the blue frame exactly over the capsule's own 2 px white border. */
    sgl_obj_set_size(menu_highlight,
                     cap->coords.x2 - cap->coords.x1 + 5,
                     cap->coords.y2 - cap->coords.y1 + 5);

    /* x follows the capsule directly (page-local x differs by 1 px); y
     * either jumps (page entry / cross-page landing) or glides. All
     * positioning uses absolute coords (see anim callbacks above), and
     * while the page itself is sliding in we only place, never animate. */
    tx = cap->coords.x1 - 2;
    ty = cap->coords.y1 - 2;
    sgl_anim_delete_by_obj(menu_highlight); /* restart cleanly on rapid moves */
    if (!animate || ty == menu_highlight->coords.y1 ||
        sgl_anim_get_by_obj(sgl_screen_act()) != NULL)
    {
        sgl_obj_set_abs_pos(menu_highlight, tx, ty);
    }
    else
    {
        sgl_obj_set_abs_pos(menu_highlight, tx, menu_highlight->coords.y1);
        sgl_anim_setup_obj_action(menu_highlight, menu_highlight->coords.y1, ty,
                                  150, anim_hl_slide_cb,
                                  SGL_ANIM_PATH_EASE_OUT_QUART, NULL);
    }
}
#endif /* MENU_HIGHLIGHT_ANIM */

/* (re)enter a menu page. A plain entry (SET long-press) resets the
 * selection to the page's first item; a page-crossing move keeps the
 * global index (menu_keep_index) so navigation continues seamlessly. */
static void menu_enter(void)
{
    sgl_obj_t ***items;
    uint8_t count;
    uint8_t start;
    uint8_t i;

    items = menu_get_items(&count);
    if (items == NULL)
    {
        return;
    }
    start = (cur_page_id == UI_SCREEN2) ? MENU_PAGE2_START : 0;
    if (!menu_keep_index || menu_index < start || menu_index >= start + count)
    {
        menu_index = start;
    }
    menu_keep_index = false;
    for (i = 0; i < count; i++)
    {
        menu_apply_style(start + i, false);
    }
#if MENU_HIGHLIGHT_ANIM
    menu_highlight_sync(false); /* place frame on the selected item, no slide */
#else
    menu_apply_style(menu_index, true); /* legacy colour-swap selection */
#endif
}

/* move the selection one step, wrapping around the 9-item virtual list:
 * crossing a page boundary rebuilds the other page and continues there
 * (legacy behaviour). delta: +1 = down (reduce key), -1 = up (add key). */
static void menu_step(int8_t delta)
{
    sgl_obj_t ***items;
    uint8_t count;
    uint8_t start;
    uint8_t cur_page, tgt_page;
    int32_t ni;

    items = menu_get_items(&count);
    if (items == NULL)
    {
        return; /* not on a menu page: ignore */
    }
    start = (cur_page_id == UI_SCREEN2) ? MENU_PAGE2_START : 0;
    cur_page = (start == 0) ? UI_SCREEN1 : UI_SCREEN2;

    ni = (int32_t)menu_index + delta;
    if (ni < 0)
    {
        ni += MENU_ITEM_TOTAL; /* up past screen1's first item: screen2's last */
    }
    if (ni >= MENU_ITEM_TOTAL)
    {
        ni -= MENU_ITEM_TOTAL; /* down past screen2's last item: screen1's first */
    }
    if ((uint8_t)ni == menu_index)
    {
        return;
    }
    tgt_page = (ni < MENU_PAGE2_START) ? UI_SCREEN1 : UI_SCREEN2;
    if (tgt_page != cur_page)
    {
        /* crossing pages: the old page's styles die with it, just load the
         * target page; its on_enter -> menu_enter() keeps menu_index and
         * highlights the landed item */
        menu_index = (uint8_t)ni;
        menu_keep_index = true;
        ui_page_load(tgt_page);
    }
    else
    {
#if MENU_HIGHLIGHT_ANIM
        menu_index = (uint8_t)ni;
        menu_highlight_sync(true); /* slide the frame to the new item */
#else
        menu_apply_style(menu_index, false);
        menu_index = (uint8_t)ni;
        menu_apply_style(menu_index, true);
#endif
    }
}

/* input-task side: single-byte flag writes, atomically consumed by
 * ui_handle() in the output task */
static volatile uint8_t menu_down_req = 0;
static volatile uint8_t menu_up_req = 0;

void ui_menu_move_request(int8_t dir)
{
    if (dir > 0)
    {
        menu_down_req = 1;
    }
    else if (dir < 0)
    {
        menu_up_req = 1;
    }
}

static void screen1_on_enter(void)
{
    /* bottom decoration bar 240x29 (legacy picture id 57) */
    ui_pic_bind(img_ext0, PIC_BOTTOM_BAR_CN);
    menu_enter(); /* highlight the first item on entry */
}

static void screen2_on_enter(void)
{
    /* bottom decoration bar 240x29 (legacy picture id 57) */
    ui_pic_bind(img_ext1, PIC_BOTTOM_BAR_CN);
    menu_enter(); /* highlight the first item on entry */
}

/* app-layer page loader: on_enter runs after the page is (re)built, for
 * rebinding dynamic content (flash pictures, focus-dependent stuff) */
static const struct
{
    ui_scr_id_t id;
    sgl_obj_t *(*init)(void);
    void (*on_enter)(void);
} ui_pages_local[] = {
    {UI_SCREEN0, ui_screen0_init, NULL},
    {UI_SCREEN1, ui_screen1_init, screen1_on_enter},
    {UI_SCREEN2, ui_screen2_init, screen2_on_enter},
};

void ui_page_load(ui_scr_id_t id)
{
    sgl_obj_t *old_page = sgl_screen_act();
    sgl_obj_t *new_page = ui_pages_local[id].init();

    /* sgl_obj_init(NULL) attaches the new page to the current active page
     * (sgl_core.c:994: parent = sgl_screen_act()), i.e. the page we are
     * about to free. Detach it and restore the page convention: a page's
     * parent points to itself (see sgl_page_create: obj->parent = obj).
     * Without this, new_page->parent dangles at freed memory and every
     * later parent->coords read (set_pos/align) corrupts the object tree. */
    if (new_page->parent != new_page)
    {
        sgl_obj_remove(new_page);
        new_page->parent = new_page;
        /* The dirty-area harvest clips every dirty object against its
         * parent's fill_rect = parent->coords ∩ parent->area. A freshly
         * malloc'd page has area = (0,0,0,0), so with parent == itself the
         * clip shrinks to 1 pixel and the new page is never redrawn (the
         * screen appears frozen). Seed the page's area with its coords so
         * the harvest sees a full-screen parent fill. */
        new_page->area = new_page->coords;
    }

    sgl_screen_load(new_page);
    cur_page_id = id; /* single source of truth, updated on every load */

    if (old_page != NULL && old_page != new_page)
    {
#if MENU_HIGHLIGHT_ANIM
        /* detach the highlight frame's slide animation first: the frame
         * dies with the page, and a still-running animation would tick
         * against freed memory (same pattern as toast/spinner DESTROYED) */
        if (menu_highlight != NULL)
        {
            sgl_anim_delete_by_obj(menu_highlight);
            menu_highlight = NULL;
        }
#endif
        sgl_obj_free(old_page);
    }

    if (ui_pages_local[id].on_enter != NULL)
    {
        ui_pages_local[id].on_enter();
    }
}

/* long-press SET: toggle page 0 <-> page 1.
 * page_free/page_load touch the SGL object tree: they must run in the UI
 * (output) task context, never concurrently with sgl_task_handler().
 * Input tasks only set the request flag; ui_handle() executes it. */
static volatile uint8_t page_toggle_req = 0;

void ui_page_toggle_request(void)
{
    page_toggle_req = 1;
}

void ui_page_toggle(void)
{
    /* page identity comes from cur_page_id, NOT from pointer comparison:
     * after a page is freed its designer global (screen0/1/2) dangles and
     * a later page can reuse that very block, making == comparisons match
     * the wrong page (seen as "long-press on page 2 went to page 1"). */
    if (cur_page_id == UI_SCREEN0)
    {
        ui_page_load(UI_SCREEN1);
    }
    else /* page 1 or page 2: back to page 0 */
    {
        ui_page_load(UI_SCREEN0);
    }
}

void ui_handle(void)
{
    if (page_toggle_req)
    {
        page_toggle_req = 0;
        ui_page_toggle();
    }

    /* menu selection movement (output task owns the object tree) */
    if (menu_down_req)
    {
        menu_down_req = 0;
        menu_step(+1);
    }
    if (menu_up_req)
    {
        menu_up_req = 0;
        menu_step(-1);
    }
}
