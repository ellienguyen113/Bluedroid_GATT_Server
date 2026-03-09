#include <stdio.h>
#include <string.h>
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

// --- SETTINGS ---
//#define MODE_SWITCH_GPIO   GPIO_NUM_5   // Physical switch to toggle modes
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

// HELPER FUNCTIOMS
static void clear_input_buffer(void)
{
    memset(input_buffer, 0, sizeof(input_buffer));
    input_count = 0;
}
static int is_valid_door(int door_id)
{
    return (door_id >= 1 && door_id <= 3);
}
static void run_door_flow(int door_id)
{
    printf("Opening Door %d...\n", door_id);
    buzzer_play_success_async();
    door_open(door_id);
    light_on(door_id);

    vTaskDelay(pdMS_TO_TICKS(5000));   // keep door open for 5 seconds

    door_close(door_id);
    light_off(door_id);
    printf("Door %d Closed.\n", door_id);
}

static int password_correct_for_selected_door(void)
{
    if (!is_valid_door(selected_door)) {
        return 0;
    }
    return (memcmp(input_buffer, PASSWORDS[selected_door - 1], PIN_LENGTH) == 0);
}

static void handle_keypad_mode(void)
{
    if (!is_valid_door(selected_door)) {
        return;
    }

    char key = get_key_buffered();

    if (key == NOPRESS) {
        return;
    }

    if (key == KEY_CLEAR) {
        clear_input_buffer();
        printf("PIN Cleared.\n");
        return;
    }

    if (key >= '0' && key <= '9') {
        if (input_count < PIN_LENGTH) {
            input_buffer[input_count] = key;
            input_count++;
            printf("Door %d PIN digit %d/%d\n", selected_door, input_count, PIN_LENGTH);
        }

        if (input_count == PIN_LENGTH) {
            printf("Entered PIN for Door %d: [%c%c%c%c]\n",
                   selected_door,
                   input_buffer[0], input_buffer[1], input_buffer[2], input_buffer[3]);

            if (password_correct_for_selected_door()) {
                printf("Access Granted for Door %d!\n", selected_door);
                run_door_flow(selected_door);
            } else {
                printf("Access Denied for Door %d!\n", selected_door);
                buzzer_play_failure_async();
            }

            clear_input_buffer();
        }
    }
}

static void handle_auto_mode_all_doors(void)
{
    float d1 = ultrasonic_get_distance_cm(1);
    vTaskDelay(pdMS_TO_TICKS(50));

    float d2 = ultrasonic_get_distance_cm(2);
    vTaskDelay(pdMS_TO_TICKS(50));

    float d3 = ultrasonic_get_distance_cm(3);
    vTaskDelay(pdMS_TO_TICKS(50));

    // ---------------- Door 1 ----------------
    if (d1 > 0 && d1 < 30.0f) {

        printf("Person detected at Door 1\n");

        door_open(1);
        light_on(1);
        buzzer_play_success_async();

        // wait until person leaves
        while (1) {
            float d = ultrasonic_get_distance_cm(1);
            if (d < 0 || d >= 35.0f) {
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        printf("Person left Door 1 area\n");

        // wait 5 seconds before closing
        vTaskDelay(pdMS_TO_TICKS(5000));

        door_close(1);
        light_off(1);
    }

    // ---------------- Door 2 ----------------
    if (d2 > 0 && d2 < 30.0f) {

        printf("Person detected at Door 2\n");

        door_open(2);
        light_on(2);
        buzzer_play_success_async();

        while (1) {
            float d = ultrasonic_get_distance_cm(2);
            if (d < 0 || d >= 35.0f) {
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        printf("Person left Door 2 area\n");

        vTaskDelay(pdMS_TO_TICKS(5000));

        door_close(2);
        light_off(2);
    }

    // ---------------- Door 3 ----------------
    if (d3 > 0 && d3 < 30.0f) {

        printf("Person detected at Door 3\n");

        door_open(3);
        light_on(3);
        buzzer_play_success_async();

        while (1) {
            float d = ultrasonic_get_distance_cm(3);
            if (d < 0 || d >= 35.0f) {
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        printf("Person left Door 3 area\n");

        vTaskDelay(pdMS_TO_TICKS(5000));

        door_close(3);
        light_off(3);
    }
}
static void remote_door_task(void *arg){
    int door_id = (int)(intptr_t)arg;
    printf("Remote task opneing door %d...\n", door_id);
    buzzer_play_success_asugnc();
    door_open(door_id);
    light_on(door_id);

    vTaskDelay(pdMS_TO_TICKS(5000));
    door_close(door_id);
    light_off(door_id);
    printf("Remote task closed door %d.\n", door_id);
    vTaskDelete(NULL);
}
// Remote Mode: BLE-selected door opens/closes by remote_cmd
static void handle_remote_mode(void)
{
    if (!is_valid_door(selected_door)) {
        return;
    }
    if (remote_cmd == CMD_OPEN) {
        printf("Remote OPEN for Door %d\n", selected_door);
        cTasjCreate(
            remote_door_task,
            "remote_door_task",
            2048,
            (void*)(intptr_t)selected_door,
            4,
            NULL
        );
        remote_cmd = CMD_NONE;   // clear command after handling
    }
    else if (remote_cmd == CMD_CLOSE) {
        printf("Remote CLOSE for Door %d\n", selected_door);
        door_close(selected_door);
        light_off(selected_door);
        remote_cmd = CMD_NONE;   // clear command after handling
    }
}

void app_main(void)
{
    // 1. Initialize components
    bluetooth_test();    
    light_init();
    buzzer_init();
    init_keypad();
    ultrasonic_init();
    servo_init();

    // Start with everything closed
    door_close(1);
    door_close(2);
    door_close(3);
    light_off(1);
    light_off(2);
    light_off(4);

    clear_input_buffer();
    printf("System Booted.\n");
    printf("Use BLE to select:\n");
    printf("1/2/3 = Door 1/2/3\n");
    printf("4 = Auto Mode\n");
    printf("5 = Remote Mode\n");
    printf("6 = Keypad Mode\n");
    printf("7 = Open\n");
    printf("8 = Close\n");

    while (1) {
        // selected_door, selected_mode, and remote_cmd
        // are updated by BLE callback in bluetooth.c

        switch (selected_mode) {
            case MODE_AUTO:
                handle_auto_mode_all_doors();
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