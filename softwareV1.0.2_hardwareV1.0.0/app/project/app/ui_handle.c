#include "ui_handle.h"

static sgl_key_group_t *grp0;
static sgl_key_group_t *grp1;

/* app-layer page loader: replaces the generated ui_screen_load() in
 * sgl_ui/helper.c, which never frees the old page */
static const struct
{
    ui_scr_id_t id;
    sgl_obj_t *(*init)(void);
} ui_pages_local[] = {
    {UI_SCREEN0, ui_screen0_init},
    {UI_SCREEN1, ui_screen1_init},
};

void ui_page_load(ui_scr_id_t id)
{
    /* record old page BEFORE creating the new one: the new page attaches to
     * the current active page (parent == NULL), so it must be detached before
     * the old page is freed */
    sgl_obj_t *old_page = sgl_screen_act();

    sgl_obj_t *new_page = ui_pages_local[id].init();

    if (new_page->parent == old_page)
    {
        sgl_obj_remove(new_page);
    }

    sgl_screen_load(new_page);

    /* free old page immediately; sgl_obj_delete() would leak here because a
     * detached root is never revisited by the draw traversal */
    if (old_page != NULL && old_page != new_page)
    {
        sgl_obj_free(old_page);
    }
}

/* call ONCE, after ui_init() */
void ui_input_init(void)
{
    grp0 = sgl_key_group_create(1);
    sgl_key_group_add_obj(grp0, button0); /* page1 already loaded by ui_init() */
    sgl_key_group_load(grp0);
    sgl_key_down();

    grp1 = sgl_key_group_create(1); /* empty: button3 is built on demand */
}

/* re-point the group to the fresh button: the old button object was freed
 * together with its page, so the stored pointer is stale */
static void key_group_focus(sgl_key_group_t *grp, sgl_obj_t *btn)
{
    grp->obj[0] = btn;
    grp->focused = -1;
    sgl_key_group_load(grp);
    sgl_key_down();
}

/* strong overrides of sgl_ui/event.c weak handlers */
void button0_released(sgl_event_t *e)
{
    (void)e;
    ui_page_load(UI_SCREEN1); /* build page2, switch active, free page1 */
    key_group_focus(grp1, button3);
}

void button3_released(sgl_event_t *e)
{
    (void)e;
    ui_page_load(UI_SCREEN0);
    key_group_focus(grp0, button0);
}

/* physical key feed - called from output task */
volatile uint8_t ui_key_evt = 0; /* 0=none 1=pressed 2=released */

void ui_key_poll(void)
{
    if (ui_key_evt == 1)
    {
        sgl_key_enter_pressed();
    }
    else if (ui_key_evt == 2)
    {
        sgl_key_enter_released();
    }
    ui_key_evt = 0;
}

void ui_handle(void)
{
}
