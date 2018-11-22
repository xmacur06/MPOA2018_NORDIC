#ifndef MPU6050_DRIVER__H__
#define MPU6050_DRIVER__H__

#include <stdint.h>
#include "nrf_drv_twi.h"


/* TWI instance ID. */
#define TWI_INSTANCE_ID     0

/* Device address */
#define MPU6050_ADDR        (0x68U)

/* Common register addresses definition for temperature sensor. */
#define MPU_PWR_MGMT        0x6B 
#define MPU_ACC_X_H         0x3BU
#define MPU_ACC_X_L         0x3CU
#define MPU_ACC_Y_H         0x3DU
#define MPU_ACC_Y_L         0x3EU
#define MPU_ACC_Z_H         0x3FU
#define MPU_ACC_Z_L         0x40U
#define MPU_TEMP_OUT_H      0x41U
#define MPU_TEMP_OUT_L      0x42U
#define MPU_GYRO_X_H        0x43U
#define MPU_GYRO_X_L        0x44U
#define MPU_GYRO_Y_H        0x45U
#define MPU_GYRO_Y_L        0x46U
#define MPU_GYRO_Z_H        0x47U
#define MPU_GYRO_Z_L        0x48U

/* Mode value for MPU6050. */
#define MPU_NORMAL_MODE     0x00U
#define MPU_SLEEP_MODE      0x40U

#define MPU_LENGHT          14

/* TWI PINS CONFIG*/
#define SCL_MPU             12
#define SDA_MPU             11

typedef struct{
  int8_t  mpu_buffer[MPU_LENGHT];
  int16_t acc_x;
  int16_t acc_y;
  int16_t acc_z;
  int8_t temperature;
  int8_t frac_temperature;
  int16_t gyro_x;
  int16_t gyro_y;
  int16_t gyro_z;
} mpu_t;

typedef enum{
  NORMAL,
  SLEEP
} MPU_mode_e;


/* TWI instance. */
extern const nrf_drv_twi_t m_twi;

/* TWI working struct */
#define TWI_STRUCT_DEF(_name) \
mpu_t _name;
extern mpu_t m_twi_ws;

/**@brief Function for handling TWI events related to MPU6050.
 *
 * @details Handles all events from the TWI.
 *
 * @param[in]   p_event       Struct for indetification of event.
 * @param[in]   p_context     Pointer for handle own struct.
 */
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context);

/**@brief Function for initialization TWI0(TWI_INSTANCE_ID)
 *
 * @details Initialization TWI.
 */
void twi_init(void);

/**@brief Function for initialization MPU6050 MODE.
 *
 * @details Initialization MPU6050 MODE.
 */
void MPU6050_set_mode(MPU_mode_e const mode);


/**@brief Function for reading MPU6050 data.
 *
 * @details Read MPU6050 data.
 */
void MPU6050_read_sensor_data(void);

/**@brief Function for get Temperature.
 *
 * @details Get meas temeprature in buffer [integers].[fractional].
 *
 * @param[out]  temp_buff    Buffer for save temperature.
 */
void MPU6050_get_temperature(int8_t * temp_buff);

#endif  /* _ OUR_SERVICE_H__ */
