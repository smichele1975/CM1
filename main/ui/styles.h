#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: barStyle
lv_style_t *get_style_bar_style_MAIN_DEFAULT();
void add_style_bar_style(lv_obj_t *obj);
void remove_style_bar_style(lv_obj_t *obj);

// Style: ButtonStyle
lv_style_t *get_style_button_style_MAIN_DEFAULT();
void add_style_button_style(lv_obj_t *obj);
void remove_style_button_style(lv_obj_t *obj);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/