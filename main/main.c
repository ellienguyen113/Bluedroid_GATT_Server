#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Component APIs
#include "buzzer.h"
#include "keypad.h"
#include "ultrasonic.h"
#include "servo.h"
#include "light.h"
#include "bluetooth.h"

//SETTINGS 
#define PIN_LENGTH 4
#define KEY_CLEAR '*'
#define NOPRESS '\0'

static const char PASSWORDS[3][PIN_LENGTH] = {
    {'1', '2', '3', '4'},   // Door 1
    {'4', '5', '6', '7'},   // Door 2
    {'6', '7', '8', '9'}    // Door 3
};

static char input_buffer[PIN_LENGTH] = {0};
static int input_count = 0;

static volatile int auto_door_busy[3]   = {0, 0, 0};
static volatile int remote_door_busy[3] = {0, 0, 0};
static volatile int keypad_door_busy[3] = {0, 0, 0};

// HELPERS
static void clear_input_buffer(void)
{
    memset(input_buffer, 0, sizeof(input_buffer));
    input_count = 0;
}

static int is_valid_door(int door_id)
{
    return (door_id >= 1 && door_id <= 3);
}

static int password_correct_for_selected_door(void)
{
    if (!is_valid_door(selected_door)) {
        return 0;
    }
    return (memcmp(input_buffer, PASSWORDS[selected_door - 1], PIN_LENGTH) == 0);
}

static void run_door_flow(int door_id, bool auto_mode)
{
    buzzer_play_success_async();
    door_open(door_id);
    light_on(door_id);

    if (auto_mode) {
        while (1) {
            float d = ultrasonic_get_distance_cm(door_id);
            if (d < 0 || d >= 35.0f) {
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        printf("Person left Door %d area\n", door_id);
        vTaskDelay(pdMS_TO_TICKS(5000));
    } else {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }

    door_close(door_id);
    light_off(door_id);
}


// TASKS
static void auto_door_task(void *arg)
{
    int door_id = (int)(intptr_t)arg;
    auto_door_busy[door_id - 1] = 1;

    run_door_flow(door_id, true);

    auto_door_busy[door_id - 1] = 0;
    vTaskDelete(NULL);
}

static void remote_door_task(void *arg)
{
    int door_id = (int)(intptr_t)arg;
    remote_door_busy[door_id - 1] = 1;

    run_door_flow(door_id, false);

    remote_door_busy[door_id - 1] = 0;
    vTaskDelete(NULL);
}

static void keypad_door_task(void *arg)
{
    int door_id = (int)(intptr_t)arg;
    keypad_door_busy[door_id - 1] = 1;

    run_door_flow(door_id, false);

    keypad_door_busy[door_id - 1] = 0;
    vTaskDelete(NULL);
}

// MODE HANDLERS
static void handle_keypad_mode(void)
{
    if (!is_valid_door(selected_door)) {
        return;
    }

    int idx = selected_door - 1;
    char key = get_key_buffered();

    if (key == NOPRESS) {
        return;
    }

    if (key == KEY_CLEAR) {
        clear_input_buffer();
        return;
    }

    if (key >= '0' && key <= '9') {
        if (input_count < PIN_LENGTH) {
            input_buffer[input_count] = key;
            input_count++;
        }

        if (input_count == PIN_LENGTH) {
            if (password_correct_for_selected_door()) {
                printf("Access Granted for Door %d!\n", selected_door);

                if (!keypad_door_busy[idx]) {
                    xTaskCreate(
                        keypad_door_task,
                        "keypad_door_task",
                        3072,
                        (void *)(intptr_t)selected_door,
                        4,
                        NULL
                    );
                }
            } else {
                printf("Access Denied for Door %d!\n", selected_door);
                buzzer_play_failure_async();
            }

            clear_input_buffer();
        }
    }
}

static void handle_auto_mode(void)
{
    if (!is_valid_door(selected_door)) {
        return;
    }

    int idx = selected_door - 1;

    if (auto_door_busy[idx]) {
        return;
    }

    float d = ultrasonic_get_distance_cm(selected_door);

    if (d > 0 && d < 25.0f) {
        printf("Person detected at Door %d\n", selected_door);

        xTaskCreate(
            auto_door_task,
            "auto_door_task",
            3072,
            (void *)(intptr_t)selected_door,
            4,
            NULL
        );
    }
}

static void handle_remote_mode(void)
{
    if (!is_valid_door(selected_door)) {
        return;
    }

    int idx = selected_door - 1;

    if (remote_cmd == CMD_OPEN) {
        printf("Remote OPEN for Door %d\n", selected_door);

        if (!remote_door_busy[idx]) {
            xTaskCreate(
                remote_door_task,
                "remote_door_task",
                3072,
                (void *)(intptr_t)selected_door,
                4,
                NULL
            );
        }

        remote_cmd = CMD_NONE;
    }
    else if (remote_cmd == CMD_CLOSE) {
        printf("Remote CLOSE for Door %d\n", selected_door);
        door_close(selected_door);
        light_off(selected_door);
        remote_cmd = CMD_NONE;
    }
}

// MAIN
void app_main(void)
{
    bluetooth_test();
    light_init();
    buzzer_init();
    init_keypad();
    ultrasonic_init();
    servo_init();

    door_close(1);
    door_close(2);
    door_close(3);
    light_off(1);
    light_off(2);
    light_off(3);

    clear_input_buffer();

    while (1) {
        switch (selected_mode) {
            case MODE_AUTO:
                handle_auto_mode();
                break;

            case MODE_REMOTE:
                handle_remote_mode();
                break;

            case MODE_KEYPAD:
                handle_keypad_mode();
                break;

            default:
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}