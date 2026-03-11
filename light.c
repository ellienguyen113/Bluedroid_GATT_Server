#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

// LED pins for each door
#define LIGHT1_GPIO GPIO_NUM_11
#define LIGHT2_GPIO GPIO_NUM_13
#define LIGHT3_GPIO GPIO_NUM_14

void light_init(void)
{
    gpio_set_direction(LIGHT1_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(LIGHT2_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(LIGHT3_GPIO, GPIO_MODE_OUTPUT);

    gpio_set_level(LIGHT1_GPIO, 1);
    gpio_set_level(LIGHT2_GPIO, 1);
    gpio_set_level(LIGHT3_GPIO, 1);
}

void light_on(int door)
{
    switch (door) {
        case 1:
            gpio_set_level(LIGHT1_GPIO, 0);
            break;

        case 2:
            gpio_set_level(LIGHT2_GPIO, 0);
            break;

        case 3:
            gpio_set_level(LIGHT3_GPIO, 0);
            break;

        default:
            break;
    }
}

void light_off(int door)
{
    switch (door) {
        case 1:
            gpio_set_level(LIGHT1_GPIO, 1);
            break;

        case 2:
            gpio_set_level(LIGHT2_GPIO, 1);
            break;

        case 3:
            gpio_set_level(LIGHT3_GPIO, 1);
            break;

        default:
            break;
    }
}