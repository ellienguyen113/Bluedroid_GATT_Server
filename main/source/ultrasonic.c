#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "ultrasonic.h"

// Internal Global Variables (Static limits them to this file)
static esp_timer_handle_t oneshot_timer;
static uint64_t echo_pulse_time = 0;
static uint64_t rising_time = 0;

// ISR: Handles the 10us Trigger Pulse timeout
static void IRAM_ATTR oneshot_timer_handler(void* arg) {
    gpio_set_level(TRIG_PIN, 0);
}

// ISR: Handles the Echo pulse edges
static void IRAM_ATTR echo_isr_handler(void* arg) {
    int level = gpio_get_level(ECHO_PIN);
    uint64_t now = esp_timer_get_time();

    if (level == 1) {
        rising_time = now;
    } else {
        echo_pulse_time = now - rising_time;
    }
}

void ultrasonic_init(void) {

    // 1. Configure Trigger Pin
    gpio_reset_pin(TRIG_PIN);
    gpio_set_direction(TRIG_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(TRIG_PIN, 0);

    // 2. Configure Echo Pin with Interrupts
    gpio_reset_pin(ECHO_PIN);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);
    gpio_set_intr_type(ECHO_PIN, GPIO_INTR_ANYEDGE);
    
    // Note: gpio_install_isr_service should ideally be called once in app_main
    gpio_install_isr_service(0); 
    gpio_isr_handler_add(ECHO_PIN, echo_isr_handler, NULL);

    // 3. Create the One-Shot Hardware Timer
    const esp_timer_create_args_t oneshot_timer_args = {
        .callback = &oneshot_timer_handler,
        .name = "hc_sr04_trigger"
    };
    esp_timer_create(&oneshot_timer_args, &oneshot_timer);
}


float ultrasonic_get_distance_cm(void) {
    // Send 10us trigger pulse
    gpio_set_level(TRIG_PIN, 1);
    esp_timer_start_once(oneshot_timer, 10);

    // Wait for the pulse to return (max range of HC-SR04 is ~400cm, approx 24ms)
    // We wait 30ms to be safe before reading the shared global variable
    vTaskDelay(pdMS_TO_TICKS(30));

    // Calculate distance (Speed of sound constant: 58.3)
    float distance = (float)echo_pulse_time / 58.3;

    // Basic sanity check: sensor range is ~2cm to 400cm
    if (distance < 2.0 || distance > 400.0) {
        return -1.0; 
    }

    return distance;
}