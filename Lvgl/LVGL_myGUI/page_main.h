#ifndef PAGE_MAIN_H
#define PAGE_MAIN_H

#include "lvgl.h"
lv_obj_t *page_main_create(void);
void page_main_update_data(float temp, float humi);
void page_main_update_status(uint8_t connected);
void page_main_update_cache(uint16_t count);
void page_main_update_uptime(uint32_t seconds);
#endif