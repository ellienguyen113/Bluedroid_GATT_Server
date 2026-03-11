
#pragma once
#include "esp_adc/adc_oneshot.h"

void ldr_init(adc_channel_t channel);
int  ldr_read_raw(void);
bool ldr_is_dark(void);

