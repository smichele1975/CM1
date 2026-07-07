#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_SETTINGS = 2,
    _SCREEN_ID_LAST = 2
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *settings;
    lv_obj_t *top_bar;
    lv_obj_t *bottom_bar;
    lv_obj_t *setting_btn;
    lv_obj_t *label_bar;
    lv_obj_t *version_label;
    lv_obj_t *cancel_btn;
    lv_obj_t *save_btn;
    lv_obj_t *label_bar_1;
    lv_obj_t *version_label_1;
    lv_obj_t *wifi_tab;
    lv_obj_t *scan;
    lv_obj_t *password_text_area;
    lv_obj_t *ssid_name;
    lv_obj_t *ssid_text_arrea;
    lv_obj_t *brightness_slider;
    lv_obj_t *keyboard;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void create_screen_settings();
void tick_screen_settings();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/