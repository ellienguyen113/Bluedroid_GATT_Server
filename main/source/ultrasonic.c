#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "ultrasonic.h"

// ---------------- PIN ----------------
#define TRIG1_PIN GPIO_NUM_9
#define ECHO1_PIN GPIO_NUM_10

#define TRIG2_PIN GPIO_NUM_48
#define ECHO2_PIN GPIO_NUM_45  

#define TRIG3_PIN GPIO_NUM_21
#define ECHO3_PIN GPIO_NUM_47

// ---------------- INTERNAL STATE ----------------
static esp_timer_handle_t oneshot_timer_1;
static esp_timer_handle_t oneshot_timer_2;
static esp_timer_handle_t oneshot_timer_3;

static volatile uint64_t echo_pulse_time[3] = {0, 0, 0};
static volatile uint64_t rising_time[3]     = {0, 0, 0};

// ---------------- HELPERS ----------------
static gpio_num_t get_trig_pin(int sensor_id)
{
    switch (sensor_id) {
        case 1: return TRIG1_PIN;
        case 2: return TRIG2_PIN;
        case 3: return TRIG3_PIN;
        default: return GPIO_NUM_NC;
    }
}

static gpio_num_t get_echo_pin(int sensor_id)
{
    switch (sensor_id) {
        case 1: return ECHO1_PIN;
        case 2: return ECHO2_PIN;
        case 3: return ECHO3_PIN;
        default: return GPIO_NUM_NC;
    }
}

static esp_timer_handle_t get_timer_handle(int sensor_id)
{
    switch (sensor_id) {
        case 1: return oneshot_timer_1;
        case 2: return oneshot_timer_2;
        case 3: return oneshot_timer_3;
        default: return NULL;
    }
}

// ---------------- TIMER CALLBACKS ----------------
static void oneshot_timer_handler_1(void* arg)
{
    gpio_set_level(TRIG1_PIN, 0);
}

static void oneshot_timer_handler_2(void* arg)
{
    gpio_set_level(TRIG2_PIN, 0);
}

static void oneshot_timer_handler_3(void* arg)
{
    gpio_set_level(TRIG3_PIN, 0);
}

// ---------------- ECHO ISR ----------------
static void IRAM_ATTR echo_isr_handler_1(void* arg)
{
    int level = gpio_get_level(ECHO1_PIN);
    uint64_t now = esp_timer_get_time();

    if (level == 1) {
        rising_time[0] = now;
    } else {
        echo_pulse_time[0] = now - rising_time[0];
    }
}

static void IRAM_ATTR echo_isr_handler_2(void* arg)
{
    int level = gpio_get_level(ECHO2_PIN);
    uint64_t now = esp_timer_get_time();

    if (level == 1) {
        rising_time[1] = now;
    } else {
        echo_pulse_time[1] = now - rising_time[1];
    }
}

static void IRAM_ATTR echo_isr_handler_3(void* arg)
{
    int level = gpio_get_level(ECHO3_PIN);
    uint64_t now = esp_timer_get_time();

    if (level == 1) {
        rising_time[2] = now;
    } else {
        echo_pulse_time[2] = now - rising_time[2];
    }
}

// ---------------- INIT ----------------
void ultrasonic_init(void)
{
    // Trigger pins
    gpio_reset_pin(TRIG1_PIN);
    gpio_set_direction(TRIG1_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(TRIG1_PIN, 0);

    gpio_reset_pin(TRIG2_PIN);
    gpio_set_direction(TRIG2_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(TRIG2_PIN, 0);

    gpio_reset_pin(TRIG3_PIN);
    gpio_set_direction(TRIG3_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(TRIG3_PIN, 0);

    // Echo pins
    gpio_reset_pin(ECHO1_PIN);
    gpio_set_direction(ECHO1_PIN, GPIO_MODE_INPUT);
    gpio_set_intr_type(ECHO1_PIN, GPIO_INTR_ANYEDGE);

    gpio_reset_pin(ECHO2_PIN);
    gpio_set_direction(ECHO2_PIN, GPIO_MODE_INPUT);
    gpio_set_intr_type(ECHO2_PIN, GPIO_INTR_ANYEDGE);

    gpio_reset_pin(ECHO3_PIN);
    gpio_set_direction(ECHO3_PIN, GPIO_MODE_INPUT);
    gpio_set_intr_type(ECHO3_PIN, GPIO_INTR_ANYEDGE);

    // Install ISR service ONCE
    gpio_install_isr_service(0);

    gpio_isr_handler_add(ECHO1_PIN, echo_isr_handler_1, NULL);
    gpio_isr_handler_add(ECHO2_PIN, echo_isr_handler_2, NULL);
    gpio_isr_handler_add(ECHO3_PIN, echo_isr_handler_3, NULL);

    // One-shot timers
    const esp_timer_create_args_t timer1_args = {
        .callback = &oneshot_timer_handler_1,
        .name = "hc_sr04_trigger_1"
    };
    esp_timer_create(&timer1_args, &oneshot_timer_1);

    const esp_timer_create_args_t timer2_args = {
        .callback = &oneshot_timer_handler_2,
        .name = "hc_sr04_trigger_2"
    };
    esp_timer_create(&timer2_args, &oneshot_timer_2);

    const esp_timer_create_args_t timer3_args = {
        .callback = &oneshot_timer_handler_3,
        .name = "hc_sr04_trigger_3"
    };
    esp_timer_create(&timer3_args, &oneshot_timer_3);
}

// ---------------- READ DISTANCE ----------------
float ultrasonic_get_distance_cm(int sensor_id)
{
    gpio_num_t trig = get_trig_pin(sensor_id);
    esp_timer_handle_t timer = get_timer_handle(sensor_id);

    if (trig == GPIO_NUM_NC || timer == NULL) {
        return -1.0;
    }

    // Clear old pulse value
    echo_pulse_time[sensor_id - 1] = 0;

    // Send 10us trigger pulse
    gpio_set_level(trig, 1);
    esp_timer_start_once(timer, 10);

    // Wait for echo return
    vTaskDelay(pdMS_TO_TICKS(30));

    float distance = (float)echo_pulse_time[sensor_id - 1] / 58.3;

    if (distance < 2.0 || distance > 400.0) {
        return -1.0;
    }

    return distance;
}