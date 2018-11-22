#include <stdint.h>
#include <string.h>
#include "MPU6050_driver.h"
#include "ble_srv_common.h"
#include "app_error.h"
#include "SEGGER_RTT.h"

/* Indicates if operation on TWI has ended. */
volatile bool m_xfer_done = false;

const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
TWI_STRUCT_DEF(m_twi_ws);

/* Static functions */
static void MPU_parse_data(void);

static void MPU_parse_data(void)
{
  int16_t temp = (m_twi_ws.mpu_buffer[6] << 8) | m_twi_ws.mpu_buffer[7];
  float f_temp = temp/340.00 + 36.53;
  m_twi_ws.temperature = (int8_t)f_temp;
  m_twi_ws.frac_temperature = (int8_t)((f_temp*100.0) - (m_twi_ws.temperature*100.0));
}

void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
            if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX)
            {
              //action happen
                int16_t temp = (m_twi_ws.mpu_buffer[6] << 8) | m_twi_ws.mpu_buffer[7];
                float f_temp = temp/340.00 + 36.53;
                m_twi_ws.temperature = (int8_t)f_temp;
                m_twi_ws.frac_temperature = (int8_t)((f_temp*100.0) - (m_twi_ws.temperature*100.0));
                SEGGER_RTT_printf(0, "Temperature: %d.%d\n",m_twi_ws.temperature, m_twi_ws.frac_temperature); 
            }
            m_xfer_done = true;
            break;
        default:
            break;
    }
}


void twi_init(void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_mpu_config = {
       .scl                = SCL_MPU,
       .sda                = SDA_MPU,
       .frequency          = NRF_DRV_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_mpu_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

void MPU6050_set_mode(MPU_mode_e const mode)
{
    ret_code_t err_code;

    /* Writing to MPU_PWR_MGMT value "0" for NORMAL mode. */
    uint8_t reg[2] = {MPU_PWR_MGMT, MPU_NORMAL_MODE};

    if(mode == SLEEP)
    {
        /* Writing to MPU_PWR_MGMT switch to SLEEP mode. */
        reg[1] = MPU_SLEEP_MODE;
    }
    
    err_code = nrf_drv_twi_tx(&m_twi, MPU6050_ADDR, reg, sizeof(reg), false);
    APP_ERROR_CHECK(err_code);
    while (m_xfer_done == false);
}

void MPU6050_read_sensor_data(void)
{ 
    ret_code_t err_code;
    /* Set start of reading TWI */
    uint8_t reg[1] = {MPU_ACC_X_H};
    m_xfer_done = false;
    err_code = nrf_drv_twi_tx(&m_twi, MPU6050_ADDR, reg, sizeof(reg), false);
    APP_ERROR_CHECK(err_code);
    while (m_xfer_done == false);
    /* Read 14 bytes from the specified address. */
    err_code = nrf_drv_twi_rx(&m_twi, MPU6050_ADDR, m_twi_ws.mpu_buffer, sizeof(m_twi_ws.mpu_buffer));
    APP_ERROR_CHECK(err_code);
}

void MPU6050_get_temperature(int8_t * temp_buff)
{
  *temp_buff++ = m_twi_ws.temperature;
  *temp_buff   = m_twi_ws.frac_temperature;
}