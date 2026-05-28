/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "waveshare_rgb_lcd_port.h"
#include "student_app.h"

void app_main()
{
    waveshare_esp32_s3_rgb_lcd_init();
    wavesahre_rgb_lcd_bl_on();

    ESP_LOGI(TAG, "Launch Student App");
    if (lvgl_port_lock(-1)) {
        student_app_create();
        lvgl_port_unlock();
    }
}
