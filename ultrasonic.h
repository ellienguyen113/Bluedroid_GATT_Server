#pragma once
#include "driver/gpio.h"


// Function Prototypes
/*
 * @brief Initializes the GPIOs, Interrupts, and Hardware Timers for the sensor.
 */
void ultrasonic_init(void);

/*
 * @brief Triggers a measurement and returns the distance in centimeters.
 * @return float Distance in cm. Returns -1.0 if out of range or error.
 */
float ultrasonic_get_distance_cm(int sensor_id);
