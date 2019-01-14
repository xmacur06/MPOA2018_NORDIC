#include <stdint.h>
#include <string.h>
#include "data_service.h"
#include "ble_srv_common.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "SEGGER_RTT.h"

/*************************** Define macros *************************/
/*******************************************************************/
#define LED2  18
#define LED3  19
#define LED4  20
#define LED2_MASK 1
#define LED3_MASK 2
#define LED4_MASK 4
#define SET_LED   0
#define CLEAR_LED 1
/*******************************************************************/
BLE_DATA_DEF(m_data_service);                                                   /**<Service module>*/

/************** Declarations of static functions *******************/
/*******************************************************************/
static void ble_uart_tranciever_handler(uint8_t const * data, uint8_t len);
static void ble_state_led_handler(uint8_t const * data, uint8_t len, ble_service_data_t * p_data_service);
static void ble_data_char_handler(ble_gatts_evt_write_t const  * p_evt_write, ble_service_data_t * p_data_service, uint8_t const * data,  uint8_t len);
static uint32_t uart_char_add(ble_service_data_t * p_data_service);
static uint32_t led_char_add(ble_service_data_t * p_data_service);
/*******************************************************************/


/**@brief Function for handle UART characteristic 
 *
 * @param[in]   data               Pointer to data.
 * @param[in]   len                Length of data buffer.
 *
 */
static void ble_uart_tranciever_handler(uint8_t const * data, uint8_t len)
{
    uint32_t err_code;
    for (uint32_t i = 0; i < len; i++)
    {
        do
        {
            err_code = app_uart_put(data[i]);
            if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_BUSY))
            {
                APP_ERROR_CHECK(err_code);
            }
        } while (err_code == NRF_ERROR_BUSY);
    }
        while (app_uart_put('\r') == NRF_ERROR_BUSY);
        while (app_uart_put('\n') == NRF_ERROR_BUSY);
}

/**@brief Function for handle LED characteristic 
 *
 * @param[in]   p_data_service     Pointer to data service struct.
 * @param[in]   data               Pointer to data.
 * @param[in]   len                Length of data buffer.
 *
 */
static void ble_state_led_handler(uint8_t const * data, uint8_t len, ble_service_data_t * p_data_service)
{
  //Clear LED
  if((data[0] >> 4) == 0)
  {
    if(data[0] & LED2_MASK) nrf_gpio_pin_write(LED2, CLEAR_LED);
    if(data[0] & LED3_MASK) nrf_gpio_pin_write(LED3, CLEAR_LED);
    if(data[0] & LED4_MASK) nrf_gpio_pin_write(LED4, CLEAR_LED);
  }
  //Set LED
  else if((data[0] >> 4) == 1)
  {
    if(data[0] & LED2_MASK) nrf_gpio_pin_write(LED2, SET_LED);
    if(data[0] & LED3_MASK) nrf_gpio_pin_write(LED3, SET_LED);
    if(data[0] & LED4_MASK) nrf_gpio_pin_write(LED4, SET_LED);
  }
  //Toggle LED
  else if((data[0] >> 4) == 2)
  {
    if(data[0] & LED2_MASK) nrf_gpio_pin_toggle(LED2);
    if(data[0] & LED3_MASK) nrf_gpio_pin_toggle(LED3);
    if(data[0] & LED4_MASK) nrf_gpio_pin_toggle(LED4);
  }
  else if((data[0] >> 4) == 3)
  {
    SEGGER_RTT_WriteString(0, "pokus.\n");
    led_characteristic_update(p_data_service);
  }
}

/**@brief Function for recognization of coming request 
 *
 * @param[in]   p_evt_write        Pointer to struct with handle number.
 * @param[in]   p_data_service     Pointer to data service struct.
 * @param[in]   data               Pointer to data.
 * @param[in]   len                Length of data buffer.
 *
 */
static void ble_data_char_handler(ble_gatts_evt_write_t const * p_evt_write, ble_service_data_t * p_data_service, uint8_t const * data,  uint8_t len)
{
  if(p_evt_write->handle == p_data_service->char_uart_handle.value_handle)
  {
    SEGGER_RTT_WriteString(0, "Service Data uart chara.\n");
    ble_uart_tranciever_handler(data, len);
  }
  else if(p_evt_write->handle == p_data_service->char_led_handle.value_handle)
  {
    SEGGER_RTT_WriteString(0, "Service Data LED chara.\n");
    ble_state_led_handler(data, len, p_data_service);
  }
  else if(p_evt_write->handle == p_data_service->char_uart_handle.cccd_handle)                                    
  {
    SEGGER_RTT_printf(0, "Service Data else chara %d.\n",  p_evt_write->handle);
    SEGGER_RTT_printf(0, "CCCD UART else chara %d.\n",  p_data_service->char_uart_handle.cccd_handle);
    if(data[0] == 1)
    {
      SEGGER_RTT_WriteString(0, "Notification enabled.\n");
      p_data_service->fs_t.f_notifi_UART_ON = true;
    }
    else if(data[0] == 2)
    {
      SEGGER_RTT_WriteString(0, "Indication enabled.\n");
      p_data_service->fs_t.f_indic_LED_ON = true;
    }
    else
    {
      p_data_service->fs_t.f_indic_UART_ON = false;
      p_data_service->fs_t.f_notifi_UART_ON = false;
      SEGGER_RTT_WriteString(0, "UART Indication/Notification disabled enabled.\n");
    } 
  }
  else if(p_evt_write->handle == p_data_service->char_led_handle.cccd_handle)
  {
    SEGGER_RTT_printf(0, "Service Data else chara %d.\n",  p_evt_write->handle);
    SEGGER_RTT_printf(0, "CCCD LED else chara %d.\n",  p_data_service->char_led_handle.cccd_handle);
    if(data[0] == 1)
    {
      SEGGER_RTT_WriteString(0, "Notification enabled.\n");
      p_data_service->fs_t.f_notifi_LED_ON = true;
    }
    else if(data[0] == 2)
    {
      SEGGER_RTT_WriteString(0, "Indication enabled.\n");
      p_data_service->fs_t.f_indic_LED_ON = true;
    }
    else
    {
      p_data_service->fs_t.f_indic_LED_ON = false;
      p_data_service->fs_t.f_notifi_LED_ON = false;
      SEGGER_RTT_WriteString(0, "LED Indication/Notification disabled enabled.\n");
      //SEGGER_RTT_printf(0, "CCCD value %d.\n",  data[0]); 
      //0x0001 (Notification)
      //0x0002 (Indication)
    } 
  }
}



/**@brief Function that will take care of some housekeeping of ble connections related to our service and characteristic 
 *
 * @param[in]   p_ble_evt        Pointer to ble_event_struct.
 * @param[in]   p_context        Pointer to our service struct.
 *
 */
void ble_data_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
  ble_service_data_t * p_data_service =(ble_service_data_t *) p_context; 
  if (p_data_service == NULL || p_ble_evt == NULL)
  {
    return;
  }
  // Switch case handling BLE events related to our service.
  switch (p_ble_evt->header.evt_id)
  {
    case BLE_GAP_EVT_CONNECTED:
      p_data_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
      SEGGER_RTT_WriteString(0, "Data service connected.\n");
      break;

    case BLE_GAP_EVT_DISCONNECTED:
      p_data_service->conn_handle = BLE_CONN_HANDLE_INVALID;
      SEGGER_RTT_WriteString(0, "Data service disconnected.\n");
      break;

    case BLE_GATTS_EVT_WRITE:
      ble_data_char_handler(&p_ble_evt->evt.gatts_evt.params.write, p_data_service, p_ble_evt->evt.gatts_evt.params.write.data, p_ble_evt->evt.gatts_evt.params.write.len);
      break;

    //Handle value confirmation for indication
    //Confirmation received from the peer.
    case BLE_GATTS_EVT_HVC:
      SEGGER_RTT_WriteString(0, "UART confirmed.\n");
      break;

    //Handle value confirmation for notification
    //Notification transmission complete.
    case BLE_GATTS_EVT_HVN_TX_COMPLETE:
      SEGGER_RTT_WriteString(0, "LED confirmed.\n");
      break;

     default:
         // No implementation needed.
         break;
 }
        	
	
}

/**@brief Function for adding our new characterstic to "Our service" that we initiated in the previous tutorial. 
 *
 * @param[in]   p_data_service        Our Service structure.
 *
 */
static uint32_t uart_char_add(ble_service_data_t * p_data_service)
{
    //Add a custom characteristic UUID
    uint32_t            err_code;
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = BLE_UUID_OUR_BASE_UUID;
    char_uuid.uuid                = BLE_UUID_UART_CHARACTERISTC_UUID;
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
    //char_md.char_props.notify   = 1;
    char_md.char_props.indicate = true;
       
    
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
    attr_char_value.max_len     = UART_MAX_LEN;
    attr_char_value.init_len    = 5;
    uint8_t welcome_msg[5]      = {'M','P','O','A','\n'};
    attr_char_value.p_value     = welcome_msg;


    //Add our new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_data_service->service_handle,
                                   &char_md,
                                   &attr_char_value,
                                   &p_data_service->char_uart_handle);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}



/**@brief Function for adding our new characterstic to "Our service" that we initiated in the previous tutorial. 
 *
 * @param[in]   ble_service_data_t        Our Service structure.
 *
 */
static uint32_t led_char_add(ble_service_data_t * p_data_service)
{
    //Add a custom characteristic UUID
    uint32_t            err_code;
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = BLE_UUID_OUR_BASE_UUID;
    char_uuid.uuid                = BLE_UUID_LED_CHARACTERISTC_UUID;
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
    attr_char_value.max_len     = 1;
    attr_char_value.init_len    = 1;
    uint8_t value               = 0x00;
    attr_char_value.p_value     = &value;


    //Add our new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_data_service->service_handle,
                                   &char_md,
                                   &attr_char_value,
                                   &p_data_service->char_led_handle);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

/**@brief Function for initiating our new service.
 *
 * @param[in]   p_our_service        Our Service structure.
 *
 */
void data_service_init(ble_service_data_t * p_data_service)
{
    // Declare 16 bit service and 128 bit base UUIDs and add them to BLE stack table     
      uint32_t   err_code;
      ble_uuid_t        service_uuid;
      ble_uuid128_t     base_uuid = BLE_UUID_OUR_BASE_UUID;
      service_uuid.uuid           = BLE_UUID_DATA_SERVICE;
      err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);
      APP_ERROR_CHECK(err_code);

      p_data_service->conn_handle = BLE_CONN_HANDLE_INVALID;
          
    // Add our service
      err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                    &service_uuid,
                                    &p_data_service->service_handle);
      APP_ERROR_CHECK(err_code);

      //Add Characteristic
       err_code = uart_char_add(p_data_service);
       APP_ERROR_CHECK(err_code);

       err_code = led_char_add(p_data_service);
       APP_ERROR_CHECK(err_code);

    // Print messages to Segger Real Time Terminal
    // UNCOMMENT THE FOUR LINES BELOW AFTER INITIALIZING THE SERVICE OR THE EXAMPLE WILL NOT COMPILE.
    SEGGER_RTT_WriteString(0, "Executing data_service_init().\n"); // Print message to RTT to the application flow
    SEGGER_RTT_printf(0, "Service UUID: 0x%#04x\n", service_uuid.uuid); // Print service UUID should match definition BLE_UUID_OUR_SERVICE
    SEGGER_RTT_printf(0, "Service UUID type: 0x%#02x\n", service_uuid.type); // Print UUID type. Should match BLE_UUID_TYPE_VENDOR_BEGIN. Search for BLE_UUID_TYPES in ble_types.h for more info
    SEGGER_RTT_printf(0, "Service handle: 0x%#04x\n", p_data_service->service_handle); // Print out the service handle. Should match service handle shown in MCP under Attribute values
}


/**@brief Function to be called when updating characteristic value
 *
 * @param[in]   p_our_service        Our Service structure.
 *
 */
void led_characteristic_update(ble_service_data_t *p_our_service)
{
    // Update characteristic value
    //Check if Connected and if notification is ON
    if ((p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID) && 
        (p_our_service->fs_t.f_notifi_LED_ON == true))
    {
        uint32_t  err_code;
        uint16_t  len = 1;
        uint32_t  port_state = nrf_gpio_port_out_read(NRF_GPIO);
        uint8_t   data = (~(port_state >> 18)) & 7;
        ble_gatts_hvx_params_t hvx_params;
        memset(&hvx_params, 0, sizeof(hvx_params));
        
        hvx_params.handle = p_our_service->char_led_handle.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        //hvx_params.type   = BLE_GATT_HVX_INDICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &len;
        hvx_params.p_data = &data;  
        
        err_code = sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);
        SEGGER_RTT_printf(0, "HVX led status: %d\n", err_code);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function to be called when updating characteristic value
 *
 * @param[in]   p_our_service        Our Service structure.
 * @param[in]   p_buff               Pointer to data buffer.
 *
 */
void uart_characteristic_update(ble_service_data_t *p_our_service, uint8_t *p_buff)
{
    //uint16_t data_len = sizeof(p_buff)/sizeof(uint8_t);  **kvuli hlavicce**
    uint16_t data_len = 18;
    if(data_len < UART_MAX_LEN - 1)
    {
       // Update characteristic value
       if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
       {
           uint32_t                err_code;
           uint16_t                len = data_len;
           ble_gatts_hvx_params_t hvx_params;
           memset(&hvx_params, 0, sizeof(hvx_params));
           
           hvx_params.handle    = p_our_service->char_uart_handle.value_handle;
           //hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
           hvx_params.type      = BLE_GATT_HVX_INDICATION;
           hvx_params.offset    = 0;
           hvx_params.p_len     = &len;
           hvx_params.p_data    = p_buff;  
           
           err_code = sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);
           SEGGER_RTT_printf(0, "HVX uart status: %d\n", err_code);
           APP_ERROR_CHECK(err_code);
       }
    }
}