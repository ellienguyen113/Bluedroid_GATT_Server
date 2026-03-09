#include "keypad.h"
#include "driver/gpio.h"
#include <string.h>
#include "freertos/FreeRTOS.h"

#define LOOP_DELAY_MS           10      // Loop sampling time (ms)
#define DEBOUNCE_TIME           40      // Debounce time (ms)
#define NROWS                   4       // Number of keypad rows
#define NCOLS                   4       // Number of keypad columns

#define ACTIVE                  0       // Keypad active state (0 = low, 1 = high)

#define NOPRESS                 '\0'    // NOPRESS character

int row_pins[] = {GPIO_NUM_3, GPIO_NUM_8, GPIO_NUM_18, GPIO_NUM_17};     // Pin numbers for rows
int col_pins[] = {GPIO_NUM_16, GPIO_NUM_15, GPIO_NUM_7, GPIO_NUM_6};   // Pin numbers for columns

char keypad_array[NROWS][NCOLS] = {

    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
void init_keypad() {
    for (int i=0; i<NROWS; i++) {
        gpio_set_direction(row_pins[i], GPIO_MODE_OUTPUT);
    }
    for (int j=0; j<NCOLS; j++) {
        gpio_set_direction(col_pins[j], GPIO_MODE_INPUT);
        if (ACTIVE == 1) {
            gpio_pulldown_en(col_pins[j]); 
        } else {
            gpio_pullup_en(col_pins[j]);
        }
    }
}

char scan_keypad() {
    char key_pressed = NOPRESS;
    for (int i=0; i<NROWS; i++) {
        gpio_set_level(row_pins[i], ACTIVE);
        for (int j=0; j<NCOLS; j++) {
            if (gpio_get_level(col_pins[j]) == ACTIVE) {
                key_pressed = keypad_array[i] [j];
            } 
        }
        gpio_set_level(row_pins[i], !ACTIVE);
    }
    return key_pressed;
}
typedef enum {
    WAIT_FOR_PRESS,
    DEBOUNCE,
    WAIT_FOR_RELEASE
} State_t;

char get_key_buffered(void) {
    static State_t state = WAIT_FOR_PRESS;
    static char candidate_key = NOPRESS; // Renamed for clarity
    static int time_ms = 0;

    char current_key = scan_keypad();
    
    switch(state) {
        case WAIT_FOR_PRESS:
            if (current_key != NOPRESS) {
                candidate_key = current_key; // Capture the NEW key
                time_ms = 0;
                state = DEBOUNCE;
            }
            break;
        
        case DEBOUNCE:
            time_ms += LOOP_DELAY_MS;
            
            // If the key changes WHILE we are debouncing, it's unstable
            if (current_key != candidate_key) {
                state = WAIT_FOR_PRESS; 
            } 
            // If it stays the SAME for the full 40ms, it's a valid press
            else if (time_ms >= DEBOUNCE_TIME) {
                state = WAIT_FOR_RELEASE;
            }
            break;

        case WAIT_FOR_RELEASE:
            if (current_key == NOPRESS) {
                state = WAIT_FOR_PRESS;
                // Return the key ONLY now. candidate_key is still '8'
                return candidate_key; 
            }
            break;
    }
    return NOPRESS;
}