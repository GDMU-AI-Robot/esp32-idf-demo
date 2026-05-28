idf_component_register(
    SRCS "waveshare_rgb_lcd_port.c" "main.c" "lvgl_port.c" "student_app.c" "lv_font_custom_16_cjk.c"
    INCLUDE_DIRS ".")

idf_component_get_property(lvgl_lib lvgl__lvgl COMPONENT_LIB)
target_compile_options(${lvgl_lib} PRIVATE -Wno-format)
