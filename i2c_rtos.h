/*
 * i2c_rtos.h
 *
 *  Created on: 15 oct. 2020
 *      Author: Emiliano
 */

#ifndef I2C_RTOS_H_
#define I2C_RTOS_H_

#include <stdint.h>

typedef enum
{
  freertos_i2c_0,
  freertos_i2c_1,
  freertos_i2c_2
} freertos_i2c_number_t;

typedef enum
{
  freertos_i2c_portA,
  freertos_i2c_portB,
  freertos_i2c_portC,
  freertos_i2c_portD,
  freertos_i2c_portE
} freertos_i2c_port_t;

typedef enum
{
  freertos_i2c_sucess,
  freertos_i2c_fail
} freertos_i2c_flag_t;

typedef struct
{
	uint32_t baudrate;
	freertos_i2c_number_t i2c_number;
	freertos_i2c_port_t port;
	uint8_t scl_pin;
	uint8_t sda_pin;
	uint8_t pin_mux;
} freertos_i2c_config_t;

typedef enum
{
	i2c_write,
	i2c_receive
}freertos_i2c_send_receive_t;

freertos_i2c_flag_t freertos_i2c_init(freertos_i2c_config_t config);

freertos_i2c_flag_t freertos_i2c_send_receive(freertos_i2c_number_t i2c_number, uint8_t * buffer, uint16_t length, uint16_t slave_addr, uint16_t subaddr, uint8_t subsize, freertos_i2c_send_receive_t i2c_send_receive);

#endif /* I2C_RTOS_H_ */
