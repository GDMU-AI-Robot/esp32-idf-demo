#ifndef STUDENT_APP_H
#define STUDENT_APP_H

#include "lvgl.h"
#include <stdbool.h>

typedef struct {
    float temperature;
    float humidity;
    int co2_ppm;
    int power_watt;
    float elec_balance;
    float water_balance;
    float elec_month_usage;
    float water_month_usage;
    bool has_alert;
} student_app_data_t;

typedef struct {
    const char *room_id;
    const char *dorm_label;
} student_app_config_t;

#define STUDENT_APP_DEFAULT_DATA() { \
    .temperature     = 25.0f,        \
    .humidity        = 60.0f,        \
    .co2_ppm         = 650,          \
    .power_watt      = 580,          \
    .elec_balance    = 32.50f,       \
    .water_balance   = 15.00f,       \
    .elec_month_usage  = 48.2f,      \
    .water_month_usage = 3.2f,       \
    .has_alert       = false         \
}

#define STUDENT_APP_DEFAULT_CONFIG() { \
    .room_id    = "302",               \
    .dorm_label = NULL                  \
}

void student_app_create(void);
void student_app_create_with_config(const student_app_config_t *cfg);

void student_app_set_data(const student_app_data_t *data);
const student_app_data_t *student_app_get_data(void);

void student_app_set_room(const char *room_id);

#endif
