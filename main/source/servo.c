#include "servo.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#define SERVO_TIMER        LEDC_TIMER_0
#define SERVO_MODE         LEDC_LOW_SPEED_MODE
#define SERVO_DUTY_RES     LEDC_TIMER_13_BIT
#define SERVO_FREQUENCY    50

#define SERVO1             2
#define SERVO2             42
#define SERVO3             41

#define SERVO1_CHANNEL     LEDC_CHANNEL_0
#define SERVO2_CHANNEL     LEDC_CHANNEL_1
#define SERVO3_CHANNEL     LEDC_CHANNEL_2

// Per-servo calibration values
static const int DUTY_CLOSE[3] = {
    200,   // Door 1 close
    200,   // Door 2 close
    190    // Door 3 close
};

static const int DUTY_OPEN[3] = {
    950,   // Door 1 open
    900,   // Door 2 open
    900    // Door 3 open
};

static ledc_channel_t get_channel_for_door(int door_id)
{
    switch (door_id) {
        case 1: return SERVO1_CHANNEL;
        case 2: return SERVO2_CHANNEL;
        case 3: return SERVO3_CHANNEL;
        default: return SERVO1_CHANNEL;
    }
}

static int get_close_duty_for_door(int door_id)
{
    switch (door_id) {
        case 1: return DUTY_CLOSE[0];
        case 2: return DUTY_CLOSE[1];
        case 3: return DUTY_CLOSE[2];
        default: return DUTY_CLOSE[0];
    }
}

static int get_open_duty_for_door(int door_id)
{
    switch (door_id) {
        case 1: return DUTY_OPEN[0];
        case 2: return DUTY_OPEN[1];
        case 3: return DUTY_OPEN[2];
        default: return DUTY_OPEN[0];
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
        .duty           = DUTY_CLOSE[0],
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&servo1));

    ledc_channel_config_t servo2 = {
        .speed_mode     = SERVO_MODE,
        .channel        = SERVO2_CHANNEL,
        .timer_sel      = SERVO_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = SERVO2,
        .duty           = DUTY_CLOSE[1],
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&servo2));

    ledc_channel_config_t servo3 = {
        .speed_mode     = SERVO_MODE,
        .channel        = SERVO3_CHANNEL,
        .timer_sel      = SERVO_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = SERVO3,
        .duty           = DUTY_CLOSE[2],
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&servo3));
}

static void servo_move_smooth(int door_id, int start, int end)
{
    ledc_channel_t channel = get_channel_for_door(door_id);
    int step = (end > start) ? 5 : -5;

    for (int d = start; d != end; d += step) {
        ledc_set_duty(SERVO_MODE, channel, d);
        ledc_update_duty(SERVO_MODE, channel);
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    ledc_set_duty(SERVO_MODE, channel, end);
    ledc_update_duty(SERVO_MODE, channel);
}

void door_open(int door_id)
{
    int close_duty = get_close_duty_for_door(door_id);
    int open_duty  = get_open_duty_for_door(door_id);

    printf("door_open(%d) -> channel %d, duty %d -> %d\n",
           door_id, get_channel_for_door(door_id), close_duty, open_duty);

    servo_move_smooth(door_id, close_duty, open_duty);
}

void door_close(int door_id)
{
    int close_duty = get_close_duty_for_door(door_id);
    int open_duty  = get_open_duty_for_door(door_id);

    printf("door_close(%d) -> channel %d, duty %d -> %d\n",
           door_id, get_channel_for_door(door_id), open_duty, close_duty);

    servo_move_smooth(door_id, open_duty, close_duty);
}