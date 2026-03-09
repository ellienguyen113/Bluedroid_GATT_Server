#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

#define LIGHT_GPIO GPIO_NUM_11

void light_init(void)
{
    gpio_set_direction(LIGHT_GPIO, GPIO_MODE_OUTPUT);
}

void light_on(void)
{
    gpio_set_level(LIGHT_GPIO, 1);
}

void light_off(void)
{
    gpio_set_level(LIGHT_GPIO, 0);
}