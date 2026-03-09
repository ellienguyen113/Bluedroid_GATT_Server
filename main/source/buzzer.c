#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "buzzer.h"

#define BUZZER_GPIO        4
#define BUZZER_TIMER       LEDC_TIMER_1
#define BUZZER_MODE        LEDC_LOW_SPEED_MODE
#define BUZZER_CHANNEL     LEDC_CHANNEL_1
#define BUZZER_DUTY_RES    LEDC_TIMER_13_BIT

void buzzer_init(void)
{
    ledc_timer_config_t timer_cfg = {
        .speed_mode       = BUZZER_MODE,
        .duty_resolution  = BUZZER_DUTY_RES,
        .timer_num        = BUZZER_TIMER,
        .freq_hz          = 2000,              // default tone (2 kHz)
        .clk_cfg          = LEDC_AUTO_CLK
    };
    (void)ledc_timer_config(&timer_cfg);

    ledc_channel_config_t ch_cfg = {
        .gpio_num   = BUZZER_GPIO,
        .speed_mode = BUZZER_MODE,
        .channel    = BUZZER_CHANNEL,
        .timer_sel  = BUZZER_TIMER,
        .duty       = 0,
        .hpoint     = 0,
        .intr_type  = LEDC_INTR_DISABLE
    };(void) ledc_channel_config(&ch_cfg);
}

void play_note(uint32_t freq_hz, uint32_t duration_ms) {
    if (freq_hz == 0) {
        // Handle a "rest" (silence)
        ledc_set_duty(BUZZER_MODE, BUZZER_CHANNEL, 0);
    } 
    else {
        ledc_set_freq(BUZZER_MODE, BUZZER_TIMER, freq_hz);
        ledc_set_duty(BUZZER_MODE, BUZZER_CHANNEL, 4096); // 50% duty for max volume
    }
    ledc_update_duty(BUZZER_MODE, BUZZER_CHANNEL);
    
    vTaskDelay(pdMS_TO_TICKS(duration_ms));

    // Brief silence between notes to make them distinct
    ledc_set_duty(BUZZER_MODE, BUZZER_CHANNEL, 0);
    ledc_update_duty(BUZZER_MODE, BUZZER_CHANNEL);
    vTaskDelay(pdMS_TO_TICKS(20)); 
}

void play_success_opening(void) {
    play_note(392, 100); 
    play_note(523, 100); 
    play_note(659, 150); 
    play_note(1046, 300); 
}
void play_failure_alarm(void) {
    for (int i = 0; i < 3; i++) {
        play_note(220, 150); // Low A (Dull buzz)
        play_note(200, 150); // Slightly lower (Discordant)
    }
    // Final long warning
    play_note(150, 600); 
}

//Internal Task Functions (Static)

static void failure_task(void *arg) {
    play_failure_alarm(); 
    vTaskDelete(NULL); // ends when the sound finishes
}

static void success_task(void *arg) {
    play_success_opening(); 
    vTaskDelete(NULL);
}

// Public API Functions

void buzzer_play_failure_async(void) {
    // We use a slightly higher priority (6) because a security 
    // failure is an urgent event.
    xTaskCreate(failure_task, "fail_snd_task", 2048, NULL, 6, NULL);
}
void buzzer_play_success_async(void) {
    xTaskCreate(success_task, "succ_snd_task", 2048, NULL, 5, NULL);
}
