/*
 * i2c_rtos.c
 *
 *  Created on: 15 oct. 2020
 *      Author: alberto
 */

#include "i2c_rtos.h"

#include "fsl_i2c.h"
#include "fsl_clock.h"
#include "fsl_port.h"

#include "FreeRTOS.h"
#include "semphr.h"

#define NUMBER_OF_SERIAL_PORTS (3)

typedef struct
{
  uint8_t is_init;
  i2c_master_handle_t fsl_i2c_handle;
  SemaphoreHandle_t mutex_tx_rx;
  SemaphoreHandle_t tx_rx_sem;
} freertos_i2c_handle_t;

static freertos_i2c_handle_t freertos_i2c_handles[NUMBER_OF_SERIAL_PORTS] = {0};

static inline void freertos_i2c_enable_port_clock(freertos_i2c_port_t port, freertos_i2c_number_t i2c_number);

static inline PORT_Type * freertos_i2c_get_port_base(freertos_i2c_port_t port);

static inline I2C_Type * freertos_i2c_get_base(freertos_i2c_number_t i2c_number);

static void fsl_i2c_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData);


static void fsl_i2c_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (kStatus_Success == status)
  {
	  if(I2C0 == base)
	  {
		  xSemaphoreGiveFromISR(freertos_i2c_handles[freertos_i2c_0].tx_rx_sem, &xHigherPriorityTaskWoken);
	  }
	  else if(I2C1 == base)
	  {
		  xSemaphoreGiveFromISR(freertos_i2c_handles[freertos_i2c_1].tx_rx_sem, &xHigherPriorityTaskWoken);
	  }
	  else if(I2C2 == base)
	  {
		  xSemaphoreGiveFromISR(freertos_i2c_handles[freertos_i2c_2].tx_rx_sem, &xHigherPriorityTaskWoken);
	  }
  }

  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

freertos_i2c_flag_t freertos_i2c_init(freertos_i2c_config_t config)
{
	freertos_i2c_flag_t retval = freertos_i2c_fail;
	i2c_master_config_t fsl_i2c_config;
	port_pin_config_t freertos_i2c_config = {kPORT_PullUp, kPORT_FastSlewRate, kPORT_PassiveFilterDisable,
			kPORT_OpenDrainDisable, kPORT_LowDriveStrength, kPORT_MuxAlt2, kPORT_UnlockRegister,};

	if(config.i2c_number < NUMBER_OF_SERIAL_PORTS)
	{
		if(!freertos_i2c_handles[config.i2c_number].is_init)
		{
			NVIC_SetPriority(I2C0_IRQn,5);
			NVIC_SetPriority(I2C1_IRQn,5);
			freertos_i2c_handles[config.i2c_number].mutex_tx_rx = xSemaphoreCreateMutex();

			freertos_i2c_handles[config.i2c_number].tx_rx_sem = xSemaphoreCreateBinary();

			/* Clock Enable */
			freertos_i2c_enable_port_clock(config.i2c_number, config.port);

			/* Port Config */
			PORT_SetPinConfig(freertos_i2c_get_port_base(config.port), config.scl_pin, &freertos_i2c_config);
			PORT_SetPinConfig(freertos_i2c_get_port_base(config.port), config.sda_pin, &freertos_i2c_config);

			I2C_MasterGetDefaultConfig(&fsl_i2c_config);
			fsl_i2c_config.baudRate_Bps = config.baudrate;
			I2C_MasterInit(freertos_i2c_get_base(config.i2c_number), &fsl_i2c_config, CLOCK_GetFreq(kCLOCK_BusClk));

			I2C_MasterTransferCreateHandle(freertos_i2c_get_base(config.i2c_number), &freertos_i2c_handles[config.i2c_number].fsl_i2c_handle, fsl_i2c_callback, NULL);

			freertos_i2c_handles[config.i2c_number].is_init = 1;

			retval = freertos_i2c_sucess;
		}
	}

	return retval;
}

freertos_i2c_flag_t freertos_i2c_send_receive(freertos_i2c_number_t i2c_number, uint8_t * buffer, uint16_t length, uint16_t slave_addr, uint16_t subaddr, uint8_t subsize, freertos_i2c_send_receive_t i2c_send_receive)
{
	freertos_i2c_flag_t flag = freertos_i2c_fail;
	i2c_master_transfer_t xfer;

	if(freertos_i2c_handles[i2c_number].is_init)
	{
		xfer.data = buffer;
		xfer.subaddress = subaddr;
		xfer.slaveAddress = slave_addr;
		xfer.subaddressSize = subsize;
		xfer.flags = kI2C_TransferDefaultFlag;
		xfer.dataSize = length;

		if(i2c_write == i2c_send_receive)
		{
			xfer.direction = kI2C_Write;
		}
		else if(i2c_receive == i2c_send_receive)
		{
			xfer.direction = kI2C_Read;
		}

		xSemaphoreTake(freertos_i2c_handles[i2c_number].mutex_tx_rx, portMAX_DELAY);
		I2C_MasterTransferNonBlocking(freertos_i2c_get_base(i2c_number), &freertos_i2c_handles[i2c_number].fsl_i2c_handle, &xfer);
		xSemaphoreTake(freertos_i2c_handles[i2c_number].tx_rx_sem, portMAX_DELAY);
		xSemaphoreGive(freertos_i2c_handles[i2c_number].mutex_tx_rx);

		flag = freertos_i2c_sucess;
	}

	return flag;
}


static inline void freertos_i2c_enable_port_clock(freertos_i2c_port_t port, freertos_i2c_number_t i2c_number)
{
	switch(port)
	{
    case freertos_i2c_portA:
      CLOCK_EnableClock(kCLOCK_PortA);
      break;
    case freertos_i2c_portB:
      CLOCK_EnableClock(kCLOCK_PortB);
      break;
    case freertos_i2c_portC:
      CLOCK_EnableClock(kCLOCK_PortC);
      break;
    case freertos_i2c_portD:
      CLOCK_EnableClock(kCLOCK_PortD);
      break;
    case freertos_i2c_portE:
      CLOCK_EnableClock(kCLOCK_PortE);
      break;
	}

	switch(i2c_number)
	{
	/**I2C 0*/
	case freertos_i2c_0:
		CLOCK_EnableClock(kCLOCK_I2c0);
		break;
		/**I2C 1*/
	case freertos_i2c_1:
		CLOCK_EnableClock(kCLOCK_I2c1);
		break;
		/**I2C 2*/
	case freertos_i2c_2:
		CLOCK_EnableClock(kCLOCK_I2c2);
		break;
	}
}

static inline PORT_Type * freertos_i2c_get_port_base(freertos_i2c_port_t port)
{
  PORT_Type * port_base = PORTA;

  switch(port)
  {
    case freertos_i2c_portA:
      port_base = PORTA;
      break;
    case freertos_i2c_portB:
      port_base = PORTB;
      break;
    case freertos_i2c_portC:
      port_base = PORTC;
      break;
    case freertos_i2c_portD:
      port_base = PORTD;
      break;
    case freertos_i2c_portE:
      port_base = PORTE;
      break;
  }

  return port_base;
}

static inline I2C_Type * freertos_i2c_get_base(freertos_i2c_number_t i2c_number)
{
	I2C_Type * retval = I2C0;

	switch(i2c_number)
	{
    case freertos_i2c_0:
      retval = I2C0;
      break;
    case freertos_i2c_1:
      retval = I2C1;
      break;
    case freertos_i2c_2:
	  retval = I2C2;
	  break;
	}

	return retval;
}
