/*=============================================================================
 * ui_handle.h - Top level UI dispatcher interface
 *   ui_handle() : periodic UI update, called from the app scheduler
 *===========================================================================*/ \
#ifndef UI_HANDLE_H
#define UI_HANDLE_H

#include "CZ01.h"
extern volatile uint8_t ui_key_evt;

void ui_handle(void);
void ui_page_load(ui_scr_id_t id);
void ui_input_init(void);
void ui_key_poll(void);

#endif