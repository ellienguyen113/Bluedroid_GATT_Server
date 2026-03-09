#include "servo.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define SERVO_TIMER        LEDC_TIMER_0
#define SERVO_MODE         LEDC_LOW_SPEED_MODE
#define SERVO_GPIO         2
#define SERVO_CHANNEL      LEDC_CHANNEL_0
#define SERVO_DUTY_RES     LEDC_TIMER_13_BIT
#define SERVO_FREQUENCY    50
#define DUTY_CLOSE_DOOR    230
#define DUTY_OPEN_DOOR     980

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

    ledc_channel_config_t servo_channel = {
        .speed_mode     = SERVO_MODE,
        .channel        = SERVO_CHANNEL,
        .timer_sel      = SERVO_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = SERVO_GPIO,
        .duty           = 0,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&servo_channel));
}

void door_open()
{
    ledc_set_duty(SERVO_MODE, SERVO_CHANNEL, DUTY_OPEN_DOOR);
    ledc_update_duty(SERVO_MODE, SERVO_CHANNEL);
}

void door_close()
{
    ledc_set_duty(SERVO_MODE, SERVO_CHANNEL, DUTY_CLOSE_DOOR);
    ledc_update_duty(SERVO_MODE, SERVO_CHANNEL);
}
