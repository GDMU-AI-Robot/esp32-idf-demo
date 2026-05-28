#include "student_app.h"
#include "lvgl_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LV_FONT_CUSTOM_16_CJK
extern const lv_font_t lv_font_custom_16_cjk;
#endif

#define BG_COLOR       0x1A1A2E
#define CARD_BG        0x252547
#define HEADER_BG      0x3B2F7C
#define TEXT_SECONDARY 0x8888AA
#define ACCENT_PURPLE  0x6C63FF
#define ACCENT_ORANGE  0xF4A261
#define ACCENT_GREEN   0x2EC4B6
#define ACCENT_BLUE    0x4361EE
#define BAR_BG         0x151530
#define STATUS_OK      0x2ECC71
#define STATUS_WARN    0xE74C3C

#define CN_FONT &lv_font_custom_16_cjk

static student_app_data_t g_data = STUDENT_APP_DEFAULT_DATA();
static student_app_config_t g_config = STUDENT_APP_DEFAULT_CONFIG();
static bool g_data_external = false;

static lv_timer_t *data_timer = NULL;

static lv_obj_t *lbl_room = NULL;
static lv_obj_t *lbl_status = NULL;
static lv_obj_t *status_dot = NULL;

static lv_obj_t *env_temp_val = NULL;
static lv_obj_t *env_temp_bar = NULL;
static lv_obj_t *env_temp_tag_lbl = NULL;

static lv_obj_t *env_humi_val = NULL;
static lv_obj_t *env_humi_bar = NULL;
static lv_obj_t *env_humi_tag_lbl = NULL;

static lv_obj_t *env_aqi_val = NULL;
static lv_obj_t *env_aqi_sub = NULL;
static lv_obj_t *env_aqi_bar = NULL;
static lv_obj_t *env_aqi_tag_lbl = NULL;

static lv_obj_t *env_power_val = NULL;
static lv_obj_t *env_power_bar = NULL;
static lv_obj_t *env_power_tag_lbl = NULL;

static lv_obj_t *bal_elec_val = NULL;
static lv_obj_t *bal_water_val = NULL;
static lv_obj_t *bal_elec_month = NULL;
static lv_obj_t *bal_water_month = NULL;

static lv_obj_t *alert_status_label = NULL;

static int prev_temp_int = 25;
static int prev_humi_int = 60;
static int prev_co2 = 650;
static int prev_power = 580;
static int prev_co2_level = 0;
static int prev_alert_level = 0;
static int prev_temp_level = 0;
static int prev_humi_level = 0;
static int prev_power_level = 0;
static float prev_elec_bal = 32.50f;
static float prev_water_bal = 15.00f;
static float prev_elec_month = 48.2f;
static float prev_water_month = 3.2f;

static void cn_txt(lv_obj_t *lbl, const char *text)
{
    lv_label_set_text(lbl, text);
}

static lv_obj_t *make_card(lv_obj_t *parent, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_remove_style_all(card);
    lv_obj_set_size(card, w, h);
    lv_obj_set_style_bg_color(card, lv_color_hex(CARD_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(card, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_all(card, 12, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return card;
}

static lv_obj_t *make_tag(lv_obj_t *parent, lv_color_t color)
{
    lv_obj_t *tag = lv_btn_create(parent);
    lv_obj_remove_style_all(tag);
    lv_obj_set_size(tag, 60, 20);
    lv_obj_set_style_bg_color(tag, color, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tag, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(tag, 10, LV_PART_MAIN);
    lv_obj_clear_flag(tag, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *lbl = lv_label_create(tag);
    lv_label_set_text(lbl, "");
    lv_obj_center(lbl);
    lv_obj_set_style_text_font(lbl, CN_FONT, 0);
    lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
    return tag;
}

static lv_obj_t *make_bar(lv_obj_t *parent, lv_color_t color)
{
    lv_obj_t *bar = lv_bar_create(parent);
    lv_bar_set_range(bar, 0, 100);
    lv_obj_set_size(bar, 150, 8);
    lv_obj_set_style_bg_color(bar, lv_color_hex(BAR_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, color, LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar, 4, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 4, LV_PART_INDICATOR);
    return bar;
}

static void create_header(lv_obj_t *parent)
{
    lv_obj_t *header = make_card(parent, 800, 56);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(HEADER_BG), LV_PART_MAIN);
    lv_obj_set_style_pad_all(header, 0, LV_PART_MAIN);

    lv_obj_t *accent = lv_obj_create(header);
    lv_obj_remove_style_all(accent);
    lv_obj_set_size(accent, 4, 28);
    lv_obj_align(accent, LV_ALIGN_LEFT_MID, 20, 0);
    lv_obj_set_style_bg_color(accent, lv_color_hex(ACCENT_PURPLE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(accent, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(accent, 2, LV_PART_MAIN);
    lv_obj_clear_flag(accent, LV_OBJ_FLAG_SCROLLABLE);

    const char *dorm_text = g_config.dorm_label ? g_config.dorm_label : "\xe5\xae\xbf\xe8\x88\x8d";
    lv_obj_t *dorm_type = lv_label_create(header);
    cn_txt(dorm_type, dorm_text);
    lv_obj_align(dorm_type, LV_ALIGN_LEFT_MID, 32, -8);
    lv_obj_set_style_text_font(dorm_type, CN_FONT, 0);
    lv_obj_set_style_text_color(dorm_type, lv_color_hex(TEXT_SECONDARY), 0);

    lbl_room = lv_label_create(header);
    lv_label_set_text(lbl_room, g_config.room_id ? g_config.room_id : "302");
    lv_obj_align(lbl_room, LV_ALIGN_LEFT_MID, 32, 8);
    lv_obj_set_style_text_font(lbl_room, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(lbl_room, lv_color_white(), 0);

    lv_obj_t *lbl_time = lv_label_create(header);
    lv_label_set_text(lbl_time, "14:32");
    lv_obj_align(lbl_time, LV_ALIGN_CENTER, 0, -8);
    lv_obj_set_style_text_font(lbl_time, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(lbl_time, lv_color_white(), 0);

    lv_obj_t *lbl_date = lv_label_create(header);
    cn_txt(lbl_date, "2026\xe5\xb9\xb4""5\xe6\x9c\x88""26\xe6\x97\xa5"" \xe6\x98\x9f\xe6\x9c\x9f\xe4\xba\x8c");
    lv_obj_align(lbl_date, LV_ALIGN_CENTER, 0, 14);
    lv_obj_set_style_text_font(lbl_date, CN_FONT, 0);
    lv_obj_set_style_text_color(lbl_date, lv_color_hex(TEXT_SECONDARY), 0);

    status_dot = lv_obj_create(header);
    lv_obj_remove_style_all(status_dot);
    lv_obj_set_size(status_dot, 10, 10);
    lv_obj_align(status_dot, LV_ALIGN_RIGHT_MID, -90, 0);
    lv_obj_set_style_bg_color(status_dot, lv_color_hex(STATUS_OK), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(status_dot, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(status_dot, 5, LV_PART_MAIN);
    lv_obj_clear_flag(status_dot, LV_OBJ_FLAG_SCROLLABLE);

    lbl_status = lv_label_create(header);
    cn_txt(lbl_status, "\xe7\xb3\xbb\xe7\xbb\x9f\xe6\xad\xa3\xe5\xb8\xb8");
    lv_obj_align(lbl_status, LV_ALIGN_RIGHT_MID, -20, 0);
    lv_obj_set_style_text_font(lbl_status, CN_FONT, 0);
    lv_obj_set_style_text_color(lbl_status, lv_color_hex(STATUS_OK), 0);
}

static lv_obj_t *env_temp_tag = NULL;
static lv_obj_t *env_humi_tag = NULL;
static lv_obj_t *env_power_tag = NULL;

static void create_env_card_simple(lv_obj_t *parent, const char *title,
                                    lv_color_t border_c,
                                    lv_obj_t **p_val, lv_obj_t **p_bar,
                                    lv_obj_t **p_tag, lv_obj_t **p_tag_lbl)
{
    lv_obj_t *card = make_card(parent, 180, 140);
    lv_obj_set_style_border_width(card, 3, LV_PART_MAIN);
    lv_obj_set_style_border_color(card, border_c, LV_PART_MAIN);
    lv_obj_set_style_border_side(card, LV_BORDER_SIDE_TOP, LV_PART_MAIN);

    lv_obj_t *t = lv_label_create(card);
    cn_txt(t, title);
    lv_obj_align(t, LV_ALIGN_TOP_MID, 0, 2);
    lv_obj_set_style_text_font(t, CN_FONT, 0);
    lv_obj_set_style_text_color(t, lv_color_hex(TEXT_SECONDARY), 0);

    *p_val = lv_label_create(card);
    lv_obj_align(*p_val, LV_ALIGN_CENTER, 0, -8);
    lv_obj_set_style_text_font(*p_val, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(*p_val, lv_color_white(), 0);

    *p_bar = make_bar(card, border_c);
    lv_obj_align(*p_bar, LV_ALIGN_BOTTOM_MID, 0, 24);

    *p_tag = make_tag(card, border_c);
    lv_obj_align(*p_tag, LV_ALIGN_BOTTOM_MID, 0, -2);
    *p_tag_lbl = lv_obj_get_child(*p_tag, 0);
}

static void create_env_row(lv_obj_t *parent)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, 770, 155);
    lv_obj_align(row, LV_ALIGN_TOP_MID, 0, 62);
    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    create_env_card_simple(row, "\xe6\xb8\xa9\xe5\xba\xa6", lv_color_hex(ACCENT_ORANGE),
                           &env_temp_val, &env_temp_bar, &env_temp_tag, &env_temp_tag_lbl);

    create_env_card_simple(row, "\xe6\xb9\xbf\xe5\xba\xa6", lv_color_hex(ACCENT_BLUE),
                           &env_humi_val, &env_humi_bar, &env_humi_tag, &env_humi_tag_lbl);

    lv_obj_t *aqi_card = make_card(row, 180, 140);
    lv_obj_set_style_border_width(aqi_card, 3, LV_PART_MAIN);
    lv_obj_set_style_border_color(aqi_card, lv_color_hex(ACCENT_GREEN), LV_PART_MAIN);
    lv_obj_set_style_border_side(aqi_card, LV_BORDER_SIDE_TOP, LV_PART_MAIN);

    lv_obj_t *aqi_title = lv_label_create(aqi_card);
    cn_txt(aqi_title, "\xe7\xa9\xba\xe6\xb0\x94\xe8\xb4\xa8\xe9\x87\x8f");
    lv_obj_align(aqi_title, LV_ALIGN_TOP_MID, 0, 2);
    lv_obj_set_style_text_font(aqi_title, CN_FONT, 0);
    lv_obj_set_style_text_color(aqi_title, lv_color_hex(TEXT_SECONDARY), 0);

    env_aqi_val = lv_label_create(aqi_card);
    cn_txt(env_aqi_val, "\xe8\x89\xaf\xe5\xa5\xbd");
    lv_obj_align(env_aqi_val, LV_ALIGN_CENTER, 0, -8);
    lv_obj_set_style_text_font(env_aqi_val, CN_FONT, 0);
    lv_obj_set_style_text_color(env_aqi_val, lv_color_hex(ACCENT_GREEN), 0);

    env_aqi_sub = lv_label_create(aqi_card);
    lv_label_set_text(env_aqi_sub, "CO2 650ppm");
    lv_obj_align(env_aqi_sub, LV_ALIGN_CENTER, 0, 18);
    lv_obj_set_style_text_font(env_aqi_sub, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(env_aqi_sub, lv_color_hex(TEXT_SECONDARY), 0);

    env_aqi_bar = make_bar(aqi_card, lv_color_hex(ACCENT_GREEN));
    lv_bar_set_value(env_aqi_bar, 65, LV_ANIM_OFF);
    lv_obj_align(env_aqi_bar, LV_ALIGN_BOTTOM_MID, 0, 24);

    lv_obj_t *aqi_tag = make_tag(aqi_card, lv_color_hex(ACCENT_GREEN));
    lv_obj_align(aqi_tag, LV_ALIGN_BOTTOM_MID, 0, -2);
    env_aqi_tag_lbl = lv_obj_get_child(aqi_tag, 0);
    cn_txt(env_aqi_tag_lbl, "\xe4\xbc\x98");

    create_env_card_simple(row, "\xe5\xbd\x93\xe5\x89\x8d\xe5\x8a\x9f\xe7\x8e\x87", lv_color_hex(ACCENT_PURPLE),
                           &env_power_val, &env_power_bar, &env_power_tag, &env_power_tag_lbl);
}

static lv_obj_t *create_balance_card(lv_obj_t *parent, const char *title)
{
    lv_obj_t *card = make_card(parent, 185, 90);

    lv_obj_t *t = lv_label_create(card);
    cn_txt(t, title);
    lv_obj_align(t, LV_ALIGN_TOP_MID, 0, 2);
    lv_obj_set_style_text_font(t, CN_FONT, 0);
    lv_obj_set_style_text_color(t, lv_color_hex(TEXT_SECONDARY), 0);

    lv_obj_t *val = lv_label_create(card);
    lv_obj_align(val, LV_ALIGN_CENTER, 0, 8);
    lv_obj_set_style_text_font(val, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(val, lv_color_white(), 0);

    return val;
}

static void create_balance_row(lv_obj_t *parent)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, 770, 105);
    lv_obj_align(row, LV_ALIGN_TOP_MID, 0, 228);
    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    bal_elec_val = create_balance_card(row, "\xe7\x94\xb5\xe8\xb4\xb9\xe4\xbd\x99\xe9\xa2\x9d");
    bal_water_val = create_balance_card(row, "\xe6\xb0\xb4\xe8\xb4\xb9\xe4\xbd\x99\xe9\xa2\x9d");
    bal_elec_month = create_balance_card(row, "\xe6\x9c\xac\xe6\x9c\x88\xe7\x94\xa8\xe7\x94\xb5");
    bal_water_month = create_balance_card(row, "\xe6\x9c\xac\xe6\x9c\x88\xe7\x94\xa8\xe6\xb0\xb4");
}

static lv_obj_t *g_mbox = NULL;

static void mbox_delete_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    g_mbox = NULL;
}

static void show_msgbox(const char *title, const char *msg, lv_color_t close_color)
{
    if (g_mbox) {
        lv_msgbox_close(g_mbox);
        g_mbox = NULL;
    }
    g_mbox = lv_msgbox_create(NULL, title, msg, NULL, true);
    lv_obj_center(g_mbox);
    lv_obj_set_style_text_font(g_mbox, CN_FONT, 0);
    lv_obj_add_event_cb(g_mbox, mbox_delete_cb, LV_EVENT_DELETE, NULL);
    lv_obj_t *close_btn = lv_msgbox_get_close_btn(g_mbox);
    if (close_btn) lv_obj_set_style_bg_color(close_btn, close_color, LV_PART_MAIN);
}

static void repair_cb(lv_event_t *e) { LV_UNUSED(e); show_msgbox("\xe6\x8f\x90\xe4\xba\xa4\xe6\x88\x90\xe5\x8a\x9f", "\xe6\x82\xa8\xe7\x9a\x84\xe6\x8a\xa5\xe4\xbf\xae\xe5\xb7\xa5\xe5\x8d\x95\n\xe5\xb7\xb2\xe6\x88\x90\xe5\x8a\x9f\xe6\x8f\x90\xe4\xba\xa4!", lv_color_hex(ACCENT_BLUE)); }
static void order_cb(lv_event_t *e) { LV_UNUSED(e); show_msgbox("\xe8\xae\xa2\xe8\xb4\xad\xe6\x88\x90\xe5\x8a\x9f", "\xe9\xa5\xae\xe7\x94\xa8\xe6\xb0\xb4\xe8\xae\xa2\xe5\x8d\x95\n\xe5\xb7\xb2\xe6\x8f\x90\xe4\xba\xa4!", lv_color_hex(ACCENT_BLUE)); }
static void topup_cb(lv_event_t *e) { LV_UNUSED(e); show_msgbox("\xe5\x85\x85\xe5\x80\xbc\xe6\x8f\x90\xe7\xa4\xba", "\xe8\xaf\xb7\xe5\x89\x8d\xe5\xbe\x80\xe7\xae\xa1\xe7\x90\x86\xe5\x91\x98\n\xe7\xab\xaf\xe8\xbf\x9b\xe8\xa1\x8c\xe5\x85\x85\xe5\x80\xbc\xe6\x93\x8d\xe4\xbd\x9c!", lv_color_hex(ACCENT_ORANGE)); }
static void alert_cb(lv_event_t *e) { LV_UNUSED(e); show_msgbox("\xe9\xa2\x84\xe8\xad\xa6\xe8\xae\xb0\xe5\xbd\x95", "\xe5\xbd\x93\xe5\x89\x8d\xe6\x97\xa0\xe6\x9c\xaa\xe5\xa4\x84\xe7\x90\x86\xe9\xa2\x84\xe8\xad\xa6,\n\xe4\xb8\x80\xe5\x88\x87\xe6\xad\xa3\xe5\xb8\xb8!", lv_color_hex(ACCENT_GREEN)); }

static void create_action_card(lv_obj_t *parent, const char *icon_sym,
                                const char *title, const char *subtitle,
                                const char *btn_text,
                                lv_color_t btn_color, lv_color_t sub_color,
                                lv_event_cb_t cb)
{
    lv_obj_t *card = make_card(parent, 182, 145);

    lv_obj_t *icon_bg = lv_obj_create(card);
    lv_obj_remove_style_all(icon_bg);
    lv_obj_set_size(icon_bg, 48, 48);
    lv_obj_align(icon_bg, LV_ALIGN_TOP_MID, 0, 4);
    lv_obj_set_style_bg_color(icon_bg, lv_color_hex(0x353560), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(icon_bg, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(icon_bg, 24, LV_PART_MAIN);
    lv_obj_clear_flag(icon_bg, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *icon = lv_label_create(icon_bg);
    lv_label_set_text(icon, icon_sym);
    lv_obj_center(icon);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(icon, btn_color, 0);

    lv_obj_t *ttl = lv_label_create(card);
    cn_txt(ttl, title);
    lv_obj_align(ttl, LV_ALIGN_TOP_MID, 0, 58);
    lv_obj_set_style_text_font(ttl, CN_FONT, 0);
    lv_obj_set_style_text_color(ttl, lv_color_white(), 0);

    lv_obj_t *sub = lv_label_create(card);
    cn_txt(sub, subtitle);
    lv_obj_align(sub, LV_ALIGN_TOP_MID, 0, 78);
    lv_obj_set_style_text_font(sub, CN_FONT, 0);
    lv_obj_set_style_text_color(sub, sub_color, 0);

    lv_obj_t *btn = lv_btn_create(card);
    lv_obj_remove_style_all(btn);
    lv_obj_set_size(btn, 130, 30);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, 4);
    lv_obj_set_style_bg_color(btn, btn_color, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 15, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *btn_txt = lv_label_create(btn);
    cn_txt(btn_txt, btn_text);
    lv_obj_center(btn_txt);
    lv_obj_set_style_text_font(btn_txt, CN_FONT, 0);
    lv_obj_set_style_text_color(btn_txt, lv_color_white(), 0);
}

static void create_action_row(lv_obj_t *parent)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, 770, 160);
    lv_obj_align(row, LV_ALIGN_TOP_MID, 0, 343);
    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    create_action_card(row, LV_SYMBOL_LEFT,
        "\xe5\x9c\xa8\xe7\xba\xbf\xe6\x8a\xa5\xe4\xbf\xae",
        "\xe6\x8f\x90\xe4\xba\xa4\xe7\xbb\xb4\xe4\xbf\xae\xe5\xb7\xa5\xe5\x8d\x95",
        "\xe7\x82\xb9\xe5\x87\xbb\xe6\x8f\x90\xe4\xba\xa4 ->",
        lv_color_hex(ACCENT_BLUE), lv_color_hex(TEXT_SECONDARY), repair_cb);

    create_action_card(row, LV_SYMBOL_TINT,
        "\xe9\xa5\xae\xe7\x94\xa8\xe6\xb0\xb4\xe8\xae\xa2\xe8\xb4\xad",
        "\xe5\x9c\xa8\xe7\xba\xbf\xe4\xb8\x8b\xe5\x8d\x95\xe9\x85\x8d\xe9\x80\x81",
        "\xe7\x82\xb9\xe5\x87\xbb\xe8\xae\xa2\xe8\xb4\xad ->",
        lv_color_hex(ACCENT_BLUE), lv_color_hex(TEXT_SECONDARY), order_cb);

    create_action_card(row, LV_SYMBOL_BELL,
        "\xe6\xb0\xb4\xe7\x94\xb5\xe5\x85\x85\xe5\x80\xbc",
        "\xe5\x9c\xa8\xe7\xba\xbf\xe5\x85\x85\xe5\x80\xbc\xe7\xbc\xb4\xe8\xb4\xb9",
        "\xe7\x82\xb9\xe5\x87\xbb\xe5\x85\x85\xe5\x80\xbc ->",
        lv_color_hex(ACCENT_ORANGE), lv_color_hex(TEXT_SECONDARY), topup_cb);

    alert_status_label = lv_label_create(parent);
    cn_txt(alert_status_label, "\xe6\x9a\x82\xe6\x97\xa0\xe5\xbc\x82\xe5\xb8\xb8");

    create_action_card(row, LV_SYMBOL_OK,
        "\xe9\xa2\x84\xe8\xad\xa6\xe8\xae\xb0\xe5\xbd\x95",
        "\xe6\x9a\x82\xe6\x97\xa0\xe5\xbc\x82\xe5\xb8\xb8",
        "\xe6\x9f\xa5\xe7\x9c\x8b\xe8\xae\xb0\xe5\xbd\x95 ->",
        lv_color_hex(ACCENT_GREEN), lv_color_hex(ACCENT_GREEN), alert_cb);
}

static void refresh_ui_from_data(void)
{
    char buf[32];

    int cur_temp_int = (int)g_data.temperature;
    if (cur_temp_int != prev_temp_int) {
        prev_temp_int = cur_temp_int;
        snprintf(buf, sizeof(buf), "%d C", cur_temp_int);
        lv_label_set_text(env_temp_val, buf);
        lv_bar_set_value(env_temp_bar, (int)((g_data.temperature - 10) / 30.0f * 100), LV_ANIM_OFF);
    }

    int cur_humi_int = (int)g_data.humidity;
    if (cur_humi_int != prev_humi_int) {
        prev_humi_int = cur_humi_int;
        snprintf(buf, sizeof(buf), "%.0f%%", g_data.humidity);
        lv_label_set_text(env_humi_val, buf);
        lv_bar_set_value(env_humi_bar, cur_humi_int, LV_ANIM_OFF);
    }

    int cur_co2_level = (g_data.co2_ppm <= 600) ? 0 : (g_data.co2_ppm <= 1000) ? 1 : 2;
    if (cur_co2_level != prev_co2_level || g_data.co2_ppm != prev_co2) {
        prev_co2_level = cur_co2_level;
        prev_co2 = g_data.co2_ppm;
        if (cur_co2_level == 0) {
            cn_txt(env_aqi_val, "\xe4\xbc\x98");
            lv_obj_set_style_text_color(env_aqi_val, lv_color_hex(ACCENT_GREEN), 0);
            lv_obj_set_style_bg_color(env_aqi_bar, lv_color_hex(ACCENT_GREEN), LV_PART_INDICATOR);
            if (env_aqi_tag_lbl) cn_txt(env_aqi_tag_lbl, "\xe4\xbc\x98");
        } else if (cur_co2_level == 1) {
            cn_txt(env_aqi_val, "\xe8\x89\xaf\xe5\xa5\xbd");
            lv_obj_set_style_text_color(env_aqi_val, lv_color_hex(ACCENT_GREEN), 0);
            lv_obj_set_style_bg_color(env_aqi_bar, lv_color_hex(ACCENT_GREEN), LV_PART_INDICATOR);
            if (env_aqi_tag_lbl) cn_txt(env_aqi_tag_lbl, "\xe4\xbc\x98");
        } else {
            cn_txt(env_aqi_val, "\xe8\xbe\x83\xe5\xb7\xae");
            lv_obj_set_style_text_color(env_aqi_val, lv_color_hex(STATUS_WARN), 0);
            lv_obj_set_style_bg_color(env_aqi_bar, lv_color_hex(STATUS_WARN), LV_PART_INDICATOR);
            if (env_aqi_tag_lbl) cn_txt(env_aqi_tag_lbl, "\xe5\xb7\xae");
        }
        snprintf(buf, sizeof(buf), "CO2 %dppm", g_data.co2_ppm);
        lv_label_set_text(env_aqi_sub, buf);
        lv_bar_set_value(env_aqi_bar, (int)(g_data.co2_ppm / 1200.0f * 100), LV_ANIM_OFF);
    }

    if (g_data.power_watt != prev_power) {
        prev_power = g_data.power_watt;
        snprintf(buf, sizeof(buf), "%d W", g_data.power_watt);
        lv_label_set_text(env_power_val, buf);
        lv_bar_set_value(env_power_bar, (int)(g_data.power_watt / 1000.0f * 100), LV_ANIM_OFF);
    }

    if (g_data.elec_balance != prev_elec_bal) {
        prev_elec_bal = g_data.elec_balance;
        snprintf(buf, sizeof(buf), "Y %.2f", g_data.elec_balance);
        lv_label_set_text(bal_elec_val, buf);
    }
    if (g_data.water_balance != prev_water_bal) {
        prev_water_bal = g_data.water_balance;
        snprintf(buf, sizeof(buf), "Y %.2f", g_data.water_balance);
        lv_label_set_text(bal_water_val, buf);
    }
    if (g_data.elec_month_usage != prev_elec_month) {
        prev_elec_month = g_data.elec_month_usage;
        snprintf(buf, sizeof(buf), "%.1f kWh", g_data.elec_month_usage);
        lv_label_set_text(bal_elec_month, buf);
    }
    if (g_data.water_month_usage != prev_water_month) {
        prev_water_month = g_data.water_month_usage;
        snprintf(buf, sizeof(buf), "%.1f m3", g_data.water_month_usage);
        lv_label_set_text(bal_water_month, buf);
    }

    int cur_alert_level = g_data.has_alert ? 1 : 0;
    if (cur_alert_level != prev_alert_level) {
        prev_alert_level = cur_alert_level;
        if (cur_alert_level) {
            cn_txt(lbl_status, "\xe6\x9c\x89\xe9\xa2\x84\xe8\xad\xa6");
            lv_obj_set_style_text_color(lbl_status, lv_color_hex(STATUS_WARN), 0);
            lv_obj_set_style_bg_color(status_dot, lv_color_hex(STATUS_WARN), LV_PART_MAIN);
            if (alert_status_label) cn_txt(alert_status_label, "\xe6\x9c\x89\xe6\x96\xb0\xe9\xa2\x84\xe8\xad\xa6");
        } else {
            cn_txt(lbl_status, "\xe7\xb3\xbb\xe7\xbb\x9f\xe6\xad\xa3\xe5\xb8\xb8");
            lv_obj_set_style_text_color(lbl_status, lv_color_hex(STATUS_OK), 0);
            lv_obj_set_style_bg_color(status_dot, lv_color_hex(STATUS_OK), LV_PART_MAIN);
            if (alert_status_label) cn_txt(alert_status_label, "\xe6\x9a\x82\xe6\x97\xa0\xe5\xbc\x82\xe5\xb8\xb8");
        }
    }

    int cur_temp_level = (g_data.temperature >= 20 && g_data.temperature <= 28) ? 0 : (g_data.temperature < 20) ? 1 : 2;
    if (cur_temp_level != prev_temp_level && env_temp_tag_lbl) {
        prev_temp_level = cur_temp_level;
        if (cur_temp_level == 0) cn_txt(env_temp_tag_lbl, "\xe9\x80\x82\xe5\xae\x9c");
        else if (cur_temp_level == 1) cn_txt(env_temp_tag_lbl, "\xe5\x81\x8f\xe5\x86\xb7");
        else cn_txt(env_temp_tag_lbl, "\xe5\x81\x8f\xe7\x83\xad");
    }

    int cur_humi_level = (g_data.humidity >= 40 && g_data.humidity <= 70) ? 0 : (g_data.humidity < 40) ? 1 : 2;
    if (cur_humi_level != prev_humi_level && env_humi_tag_lbl) {
        prev_humi_level = cur_humi_level;
        if (cur_humi_level == 0) cn_txt(env_humi_tag_lbl, "\xe9\x80\x82\xe5\xae\x9c");
        else if (cur_humi_level == 1) cn_txt(env_humi_tag_lbl, "\xe5\x81\x8f\xe5\xb9\xb2");
        else cn_txt(env_humi_tag_lbl, "\xe6\xbd\xae\xe6\xb9\xbf");
    }

    int cur_power_level = (g_data.power_watt < 600) ? 0 : 1;
    if (cur_power_level != prev_power_level && env_power_tag_lbl) {
        prev_power_level = cur_power_level;
        if (cur_power_level == 0) cn_txt(env_power_tag_lbl, "\xe6\xad\xa3\xe5\xb8\xb8");
        else cn_txt(env_power_tag_lbl, "\xe8\xbe\x83\xe9\xab\x98");
    }
}

static void sim_data_step(void)
{
    g_data.temperature += (rand() % 10 - 5) * 0.1f;
    if (g_data.temperature < 18.0f) g_data.temperature = 18.0f;
    if (g_data.temperature > 38.0f) g_data.temperature = 38.0f;

    g_data.humidity += (rand() % 10 - 5) * 0.5f;
    if (g_data.humidity < 30.0f) g_data.humidity = 30.0f;
    if (g_data.humidity > 90.0f) g_data.humidity = 90.0f;

    g_data.co2_ppm += (rand() % 100 - 50);
    if (g_data.co2_ppm < 300) g_data.co2_ppm = 300;
    if (g_data.co2_ppm > 1200) g_data.co2_ppm = 1200;

    g_data.power_watt += (rand() % 200 - 100);
    if (g_data.power_watt < 100) g_data.power_watt = 100;
    if (g_data.power_watt > 900) g_data.power_watt = 900;

    g_data.elec_balance -= 0.05f;
    if (g_data.elec_balance < 0) g_data.elec_balance = 100.0f;
    g_data.water_balance -= 0.02f;
    if (g_data.water_balance < 0) g_data.water_balance = 50.0f;
    g_data.elec_month_usage += 0.01f;
    g_data.water_month_usage += 0.001f;

    g_data.has_alert = (g_data.elec_balance < 10.0f || g_data.water_balance < 5.0f || g_data.co2_ppm > 1100);
}

static void update_data(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    if (!g_data_external) {
        sim_data_step();
    }
    refresh_ui_from_data();
}

void student_app_set_data(const student_app_data_t *data)
{
    if (data) {
        g_data = *data;
        g_data_external = true;
    } else {
        g_data_external = false;
    }
    refresh_ui_from_data();
}

const student_app_data_t *student_app_get_data(void)
{
    return &g_data;
}

void student_app_set_room(const char *room_id)
{
    if (room_id && lbl_room) {
        lv_label_set_text(lbl_room, room_id);
    }
}

static void app_init_internal(const student_app_config_t *cfg)
{
    if (cfg) {
        g_config = *cfg;
    } else {
        g_config = (student_app_config_t)STUDENT_APP_DEFAULT_CONFIG();
    }

    g_data = (student_app_data_t)STUDENT_APP_DEFAULT_DATA();
    g_data_external = false;

    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(BG_COLOR), LV_PART_MAIN);

    ((lv_font_t *)&lv_font_montserrat_12)->fallback = CN_FONT;
    ((lv_font_t *)&lv_font_montserrat_14)->fallback = CN_FONT;
    ((lv_font_t *)&lv_font_montserrat_16)->fallback = CN_FONT;
    ((lv_font_t *)&lv_font_montserrat_20)->fallback = CN_FONT;
    ((lv_font_t *)&lv_font_montserrat_24)->fallback = CN_FONT;

    create_header(scr);
    create_env_row(scr);
    create_balance_row(scr);
    create_action_row(scr);

    refresh_ui_from_data();

    data_timer = lv_timer_create(update_data, 5000, NULL);
}

void student_app_create(void)
{
    app_init_internal(NULL);
}

void student_app_create_with_config(const student_app_config_t *cfg)
{
    app_init_internal(cfg);
}
