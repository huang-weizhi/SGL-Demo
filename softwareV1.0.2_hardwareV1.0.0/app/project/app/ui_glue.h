#ifndef UI_GLUE_H
#define UI_GLUE_H

/* Handwritten glue header - force-included into every C file via Keil
 * "Misc Controls: --include ../app/ui_glue.h".
 *
 * Why this exists: the Designer-generated sgl_ui/event.c calls
 * ui_screen_load(), but no generated header declares it (the prototype
 * only lives inside ui.c). Adding it here keeps sgl_ui/ untouched, so
 * re-exporting from the Designer will never wipe this fix.
 */

#include "ui.h"

void ui_screen_load(ui_scr_id_t id);

#endif /* UI_GLUE_H */
