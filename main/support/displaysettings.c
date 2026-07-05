#include <ui/actions.h>
#include <bsp/esp32_p4_wifi6_touch_lcd_7b.h>
#include <esp_log.h>

void action_change_brightness(lv_event_t * e)   {
    lv_obj_t *obj=lv_event_get_target(e);
    int16_t val=lv_slider_get_value(obj);
    bsp_display_brightness_set(val);
}

