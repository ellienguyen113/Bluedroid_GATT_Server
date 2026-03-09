#include "servo.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define SERVO_TIMER        LEDC_TIMER_0
#define SERVO_MODE         LEDC_LOW_SPEED_MODE
#define SERVO_DUTY_RES     LEDC_TIMER_13_BIT
#define SERVO_FREQUENCY    50
#define DUTY_CLOSE_DOOR    230
#define DUTY_OPEN_DOOR     980

#define SERVO1       2
#define SERVO2       42
#define SERVO3       41

#define SERVO1_CHANNEL LEDC_CHANNEL_0
#define SERVO2_CHANNEL LEDC_CHANNEL_1
#define SERVO3_CHANNEL LEDC_CHANNEL_2

static ledc_channel_t get_channel_for_door(int door_id){
    switch(door_id){
        case 1:
        return SERVO1;
        case 2:
        return SERVO2;
        case 3:
        return SERVO3;
        default:
        return SERVO1;
    }
}

void servo_init(void)
{
    ledc_timer_config_t servo_timer = {
        .speed_mode       = SERVO_MODE,
        .duty_resolution  = SERVO_DUTY_RES,
        .timer_num        = SERVO_TIMER,
        .freq_hz          = SERVO_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&servo_timer));

    ledc_channel_config_t servo1 = {
        .speed_mode     = SERVO_MODE,
        .channel        = SERVO1_CHANNEL,
        .timer_sel      = SERVO_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = SERVO1,
        .duty           = 0,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&servo1));

    ledc_channel_config_t servo2 = {
        .speed_mode     = SERVO_MODE,
        .channel        = SERVO2_CHANNEL,
        .timer_sel      = SERVO_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = SERVO2,
        .duty           = 0,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&servo2));

    ledc_channel_config_t servo3 = {
        .speed_mode     = SERVO_MODE,
        .channel        = SERVO3_CHANNEL,
        .timer_sel      = SERVO_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = SERVO3,
        .duty           = 0,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&servo3));
}

void door_open(int door_id)
{
    ledc_channel_t channel = get_channel_for_door(door_id);
    ledc_set_duty(SERVO_MODE, channel, DUTY_OPEN_DOOR);
    ledc_update_duty(SERVO_MODE, channel);
}

void door_close(int door_id)
{
    ledc_channel_t channel = get_channel_for_door(door_id);
    ledc_set_duty(SERVO_MODE, channel, DUTY_CLOSE_DOOR);
    ledc_update_duty(SERVO_MODE, channel);
}
