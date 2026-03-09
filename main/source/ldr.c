
#include "ldr.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"

#define ADC_ATTEN     ADC_ATTEN_DB_12
#define BITWIDTH      ADC_BITWIDTH_12

#define LDR_DARK_THRESHOLD 1800 

static adc_oneshot_unit_handle_t adc1_handle;
static adc_channel_t ldr_channel;

void ldr_init(adc_channel_t channel)
{
    ldr_channel = channel;

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN,
        .bitwidth = BITWIDTH
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ldr_channel, &config));
}

int ldr_read_raw(void)
{
    int raw = 0;
    adc_oneshot_read(adc1_handle, ldr_channel, &raw);
    return raw;
}

// New function to check if light level is below threshold
bool ldr_is_dark(void)
{
    int current_level = ldr_read_raw();
    return (current_level < LDR_DARK_THRESHOLD);
}