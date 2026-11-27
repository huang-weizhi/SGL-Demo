/*=============================================================================
 * ui_handle.h - Top level UI dispatcher interface
 *   ui_handle() : periodic UI update, called from the app scheduler
 *===========================================================================*/ \
#ifndef UI_HANDLE_H
#define UI_HANDLE_H

#include "CZ01.h"

void ui_handle(void);
void ui_page_load(ui_scr_id_t id);
void ui_page_toggle(void);
void ui_page_toggle_request(void); /* call from any task; executed in ui_handle() */
void ui_menu_move_request(int8_t dir); /* +1 = selection down, -1 = up; call from input task */
#endif