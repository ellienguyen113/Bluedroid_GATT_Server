/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <stdint.h>
#include "esp_gatts_api.h"
#include "esp_gap_ble_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Bluetooth Profile Definitions */
#define PROFILE_NUM                     1
#define AUTO_IO_PROFILE_APP_ID          0
#define AUTO_IO_SVC_UUID                 0x1815  /* Automation IO Service UUID */
#define AUTO_IO_NUM_HANDLE               3

/* Pin Definitions */
#define BUZZER                           GPIO_NUM_11

/* Advertising Configuration Flags */
#define ADV_CONFIG_FLAG                  (1 << 0)
#define SCAN_RSP_CONFIG_FLAG             (1 << 1)

/* LED Status Characteristic UUID (128-bit) */
extern const uint8_t led_chr_uuid[16];

/* GATT Profile Instance Structure */
struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;              /* GATT callback function */
    uint16_t gatts_if;                      /* GATT interface number */
    uint16_t app_id;                         /* Application ID */
    uint16_t conn_id;                        /* Connection ID */
    uint16_t service_handle;              /* Service handle */
    esp_gatt_srvc_id_t service_id;     /* Service ID */
    uint16_t char_handle;                  /* Characteristic handle */
    esp_bt_uuid_t char_uuid;               /* Characteristic UUID */
    esp_gatt_perm_t perm;                   /* Characteristic permissions */
    esp_gatt_char_prop_t property;     /* Characteristic properties */
    uint16_t descr_handle;                 /* Descriptor handle */
    esp_bt_uuid_t descr_uuid;              /* Descriptor UUID */
};
/* Function Prototypes */

/*
 * @brief Initialize and start Bluetooth LE GATT server
 * 
 * This function initializes the Bluetooth controller, Bluedroid stack,
 * registers GAP and GATT callbacks, and starts the GATT server for
 * the Automation IO service.
 */
void bluetooth_init(void);

/*
 * @brief Bluetooth GAP event handler
 * 
 * @param event GAP event type
 * @param param Pointer to GAP callback parameters
 */
void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

/*
 * @brief GATT server event handler
 * 
 * @param event GATT server event type
 * @param gatts_if GATT interface
 * @param param Pointer to GATT callback parameters
 */
void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, 
                         esp_ble_gatts_cb_param_t *param);

/*
 * @brief Automation IO profile event handler
 * 
 * @param event GATT server event type
 * @param gatts_if GATT interface
 * @param param Pointer to GATT callback parameters
 */
void auto_io_gatts_profile_event_handler(esp_gatts_cb_event_t event, 
                                          esp_gatt_if_t gatts_if, 
                                          esp_ble_gatts_cb_param_t *param);

/*
 * @brief Handle write event response
 * 
 * @param gatts_if GATT interface
 * @param param Pointer to GATT callback parameters containing write event data
 */

//#ifndef BLUETOOTH_H
//#define BLUETOOTH_H
//#endif /* BLUETOOTH_H */

typedef enum {
    MODE_AUTO = 0,
    MODE_REMOTE = 1,
    MODE_KEYPAD = 2
} door_mode_t;

typedef enum {
    CMD_NONE = 0,
    CMD_OPEN = 1,
    CMD_CLOSE = 2
} remote_cmd_t;

extern volatile int selected_door;
extern volatile door_mode_t selected_mode;
extern volatile remote_cmd_t remote_cmd;

void bluetooth_test(void);


void bluetooth_test(void);


void example_write_event_env(esp_gatt_if_t gatts_if, 
                             esp_ble_gatts_cb_param_t *param);

#ifdef __cplusplus
}
#endif

#endif /* BLUETOOTH_H */