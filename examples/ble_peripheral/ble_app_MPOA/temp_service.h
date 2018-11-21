#ifndef TEMP_SERVICE_H__
#define TEMP_SERVICE_H__

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"

#define BLE_UUID_OUR_BASE_UUID              {0x23, 0xD1, 0x13, 0xEF, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00} // 128-bit base UUID
#define BLE_UUID_OUR_SERVICE1                0xAAAA // Just a random, but recognizable value

// ALREADY_DONE_FOR_YOU: Defining 16-bit characteristic UUID
#define BLE_UUID_OUR_CHARACTERISTC_UUID1     0xBBBB // Just a random, but recognizable value
#define BLE_UUID_LED_CHARACTERISTC_UUID2     0xCCCC // Just a random, but recognizable value

/**
 * @brief This structure contains various status information for our service. 
 * It only holds one entry now, but will be populated with more items as we go.
 * The name is based on the naming convention used in Nordic's SDKs. 
 * 'ble’ indicates that it is a Bluetooth Low Energy relevant structure and 
 * ‘os’ is short for Our Service). 
 */
typedef struct
{
    uint16_t                    conn_handle;        /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection).*/
    uint16_t                    service_handle;     /**< Handle of Our Service (as provided by the BLE stack). */
                                                    /**service_handle is a number identifying this particular service and is assigned by the SoftDevice**/
    ble_gatts_char_handles_t    char_handles;       /**relevant handles for our characteristic*/
    ble_gatts_char_handles_t    char_led_handles;       /**relevant handles for our characteristic*/
}ble_ts_t;

#define BLE_TS_DEF(_name) \
static ble_ts_t _name;

/**@brief Function for handling BLE Stack events related to our service and characteristic.
 *
 * @details Handles all events from the BLE stack of interest to Our Service.
 *
 * @param[in]   p_our_service       Our Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_temp_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

/**@brief Function for initializing our new service.
 *
 * @param[in]   p_our_service       Pointer to Our Service structure.
 */
void temp_service_init(ble_ts_t * p_our_service);

/**@brief Function for updating and sending new characteristic values
 *
 * @details The application calls this function whenever our timer_timeout_handler triggers
 *
 * @param[in]   p_our_service                     Our Service structure.
 * @param[in]   characteristic_value     New characteristic value.
 */
void temp_temperature_characteristic_update(ble_ts_t *p_our_service, int32_t *temperature_value);

#endif  /* _ OUR_SERVICE_H__ */
