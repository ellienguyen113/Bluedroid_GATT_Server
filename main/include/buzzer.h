#pragma once
#include "driver/ledc.h"

// Move these here so every file that #includes "buzzer.h" can see them
#define BUZZER_GPIO        4
#define BUZZER_TIMER       LEDC_TIMER_1
#define BUZZER_MODE        LEDC_LOW_SPEED_MODE
#define BUZZER_CHANNEL     LEDC_CHANNEL_1
#define BUZZER_DUTY_RES    LEDC_TIMER_13_BIT

void buzzer_init(void);
/**
 * Initialize the LEDC peripheral and PWM for the passive buzzer
 */

void play_note(uint32_t freq_hz, uint32_t duration_ms);
/*
plays a single tone and blocks the current task until finished 
*/

void play_success_opening(void);
void play_failure_alarm(void);

void buzzer_play_success_async(void);
/*
starts the success melody in a separate background task
doesn't block the calling task
*/

void buzzer_play_failure_async(void);
/*
startes the failure alarm in a separate background task */