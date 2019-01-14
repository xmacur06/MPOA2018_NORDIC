#include <stdint.h>
#include <string.h>
#include "sensor_service.h"
#include "ble_srv_common.h"
#include "app_error.h"
#include "SEGGER_RTT.h"


// ALREADY_DONE_FOR_YOU: Declaration of a function that will take care of some housekeeping of ble connections related to our service and characteristic
/**@brief Function that will take care of some housekeeping of ble connections related to our service and characteristic 
 *
 * @param[in]   p_ble_evt        Pointer to ble_event_struct.
 * @param[in]   p_context        Pointer to our service struct.
 *
 */
void ble_sensor_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
  	ble_ts_t * p_our_service =(ble_ts_t *) p_context; 
        if (p_our_service == NULL || p_ble_evt == NULL)
        {
          return;
        }
        switch (p_ble_evt->header.evt_id)
        {
            case BLE_GAP_EVT_CONNECTED:
                p_our_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
                SEGGER_RTT_WriteString(0, "Temperature service connected.\n");
                break;
            case BLE_GAP_EVT_DISCONNECTED:
                p_our_service->conn_handle = BLE_CONN_HANDLE_INVALID;
                break;
            case BLE_GATTS_EVT_WRITE:
                if(p_ble_evt->evt.gatts_evt.params.write.handle == p_our_service->char_handles.value_handle)
                {
                   
                }
                else if(p_ble_evt->evt.gatts_evt.params.write.handle == p_our_service->char_data_handles.value_handle)
                {
                    SEGGER_RTT_WriteString(0, "Read value from phone. New characteristic.\n");
                }
                else
                {
                    SEGGER_RTT_WriteString(0, "Char nesedi.\n");
                } 
                break;
            case BLE_GATTS_EVT_HVN_TX_COMPLETE:
              SEGGER_RTT_WriteString(0, "Temperature confirmed.\n");
            break;
            default:
                // No implementation needed.
                break;
        }
        	
	
}

/**@brief Function for adding our new characterstic to "Our service" that we initiated in the previous tutorial. 
 *
 * @param[in]   p_our_service        Our Service structure.
 *
 */
static uint32_t sensor_char_add(ble_ts_t * p_our_service)
{
    //Add a custom characteristic UUID
    uint32_t            err_code;
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = BLE_UUID_OUR_BASE_UUID;
    char_uuid.uuid                = BLE_UUID_LED_CHARACTERISTC_UUID2;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);  

    
    //Add read/write properties to our characteristic
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;
    char_md.char_props.write = 1;

    
    //Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc                = BLE_GATTS_VLOC_STACK;    
    char_md.p_cccd_md           = &cccd_md;
    char_md.char_props.notify   = 1;
       
    
    //Configure the attribute metadata
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc        = BLE_GATTS_VLOC_STACK; 
    
    
    //Set read/write security levels to our characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    
    //Configure the characteristic value attribute
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;

    
    //Set characteristic length in number of bytes
    attr_char_value.max_len     = 4;
    attr_char_value.init_len    = 4;
    uint8_t value[4]            = {0x11, 0x22, 0x33, 0x44};
    attr_char_value.p_value     = value;


    //Add our new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_our_service->service_handle,
                                   &char_md,
                                   &attr_char_value,
                                   &p_our_service->char_data_handles);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

/**@brief Function for initiating our new service.
 *
 * @param[in]   p_our_service        Our Service structure.
 *
 */
void sensor_service_init(ble_ts_t * p_our_service)
{
    //Declare 16 bit service and 128 bit base UUIDs and add them to BLE stack table     
      uint32_t   err_code;
      ble_uuid_t        service_uuid;
      ble_uuid128_t     base_uuid = BLE_UUID_OUR_BASE_UUID;
      service_uuid.uuid           = BLE_UUID_OUR_SERVICE1;
      err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);
      APP_ERROR_CHECK(err_code);

      p_our_service->conn_handle = BLE_CONN_HANDLE_INVALID;
          
    //Add our service
      err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                    &service_uuid,
                                    &p_our_service->service_handle);
      APP_ERROR_CHECK(err_code);

      //Add Characteristic
       err_code = sensor_char_add(p_our_service);
       APP_ERROR_CHECK(err_code);

    // Print messages to Segger Real Time Terminal
    // UNCOMMENT THE FOUR LINES BELOW AFTER INITIALIZING THE SERVICE OR THE EXAMPLE WILL NOT COMPILE.
//    SEGGER_RTT_WriteString(0, "Executing our_service_init().\n"); // Print message to RTT to the application flow
//    SEGGER_RTT_printf(0, "Service UUID: 0x%#04x\n", service_uuid.uuid); // Print service UUID should match definition BLE_UUID_OUR_SERVICE
//    SEGGER_RTT_printf(0, "Service UUID type: 0x%#02x\n", service_uuid.type); // Print UUID type. Should match BLE_UUID_TYPE_VENDOR_BEGIN. Search for BLE_UUID_TYPES in ble_types.h for more info
//    SEGGER_RTT_printf(0, "Service handle: 0x%#04x\n", p_our_service->service_handle); // Print out the service handle. Should match service handle shown in MCP under Attribute values
}


// ALREADY_DONE_FOR_YOU: Function to be called when updating characteristic value
//Toto pos�l� samo o sob� data
/**@brief Function to be called when updating characteristic value
 *
 * @param[in]   p_our_service        Our Service structure.
 * @param[in]   sensor_value         Pointer to data buffer.
 *
 */
void sensor_characteristic_update(ble_ts_t *p_our_service, uint8_t *sensor_value)
{
    //Update characteristic value
    SEGGER_RTT_printf(0, "HVX temperature status: Zkusim poslat.\n");
    if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        uint32_t                err_code;
        uint16_t                len = 4;
        ble_gatts_hvx_params_t hvx_params;
        memset(&hvx_params, 0, sizeof(hvx_params));
        
        hvx_params.handle = p_our_service->char_data_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &len;
        hvx_params.p_data = sensor_value;  
        
        sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);
        SEGGER_RTT_printf(0, "HVX temperature status: %d\n", err_code);
        APP_ERROR_CHECK(err_code);
    }
}