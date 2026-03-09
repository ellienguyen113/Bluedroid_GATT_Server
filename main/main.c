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
#include "ldr.h"
#include "light.h"
#include "bluetooth.h"


// --- SETTINGS ---
#define MODE_SWITCH_GPIO   GPIO_NUM_5   // Physical switch to toggle modes
char input_buffer[4] = {0, 0, 0, 0}; 
int input_count = 0;
char MASTER_PASSWORD[4] = {'7', '8', '9', '9'}; // The PIN
#define KEY_CLEAR '*'
#define NOPRESS '\0'


void run_door_flow(void) {
    printf("Opening Door...\n");
    buzzer_play_success_async(); // Activation tone
    door_open(); 
    if (ldr_is_dark()) {
        light_on();
    }
    else{
        light_off();
    }
    vTaskDelay(pdMS_TO_TICKS(5000)); // Stay open 5s
    door_close();
    light_off();
    printf("Door Closed.\n");
}

void app_main(void) {
    // 1. Initialize Components
    bluetooth_test();
    light_init();
    ldr_init(ADC_CHANNEL_9); 
    buzzer_init();
    init_keypad();
    ultrasonic_init();
    servo_init();

    // 2. Initialize the Mode Switch
    gpio_set_direction(MODE_SWITCH_GPIO, GPIO_MODE_INPUT);
    gpio_pullup_en(MODE_SWITCH_GPIO); // 1 = Auto, 0 = Keypad

    char input_buffer[4];
    int input_count = 0;

    printf("System Booted. Use Switch to Select Mode.\n");

    while (1) {
        // READ THE SWITCH: High (1) = Auto Mode, Low (0) = Keypad Mode
        int mode_select = gpio_get_level(MODE_SWITCH_GPIO);

        if (mode_select == 1) {
            // --- MODE 1: AUTO MODE (Ultrasonic) ---
            if (ultrasonic_get_distance_cm() < 30.0) {
                run_door_flow();
                
                // NEW: Wait until the person moves away before allowing another trigger
                printf("Waiting for person to leave...\n");
                while(ultrasonic_get_distance_cm() < 35.0) {
                    vTaskDelay(pdMS_TO_TICKS(100)); 
                }
                printf("Area clear. Monitoring resumed.\n");
            }
        }
        else {
            // --- MODE 3: KEYPAD MODE ---
            char key = get_key_buffered();
            if (key != NOPRESS) {
                if (key == KEY_CLEAR) {
                    input_count = 0;
                    printf("PIN Cleared.\n");
                } 
                else if ((key >= '0' && key <= '9')){
                    if (input_count<4){
                        input_buffer[input_count] = key;
                        input_count++;
                        printf("Digit %d/4\n", input_count);
                    }
                    if (input_count == 4) {
                        printf("DEBUG: Buffer is [%c%c%c%c]\n", 
                               input_buffer[0], input_buffer[1], input_buffer[2], input_buffer[3]);
                               
                        if (memcmp(input_buffer, MASTER_PASSWORD, 4) == 0) {
                            printf("Access Granted!\n");
                            run_door_flow();
                        } else {
                            printf("Access Denied!\n");
                            buzzer_play_failure_async(); // Warning tone
                        }
                        input_count = 0; // Reset for next try
                    }
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20)); 
    }
}
