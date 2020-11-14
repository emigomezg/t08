#include <wm8731_CODEC.h>

#define I2C_SCL		2U
#define I2C_SDA		3U
#define BAUD_RATE	100000
#define I2C_DELAY	100//(ms)

/*Global declaratios*/
static freertos_i2c_config_t WM8731_CODEC_cfg;

freertos_i2c_flag_t wm8731_init(void)
{
	freertos_i2c_flag_t wm8731_sucess = freertos_i2c_fail;
	/*Start port and freertos i2c inicialization */
	WM8731_CODEC_cfg.baudrate = BAUD_RATE;
	WM8731_CODEC_cfg.i2c_number = 	freertos_i2c_0;
	WM8731_CODEC_cfg.port = freertos_i2c_portB;

	/* Pins are setup for I2C protocol as well as the pin alt functions */
	WM8731_CODEC_cfg.scl_pin = I2C_SCL;
	WM8731_CODEC_cfg.sda_pin = I2C_SDA;
	WM8731_CODEC_cfg.pin_mux = kPORT_MuxAlt2;

	/*Init configuration for freertos i2c*/
	wm8731_sucess = freertos_i2c_init(WM8731_CODEC_cfg);
	vTaskDelay(pdMS_TO_TICKS(10));
	if(freertos_i2c_sucess == wm8731_sucess)
	{
		wm8731_sucess = freertos_i2c_fail;

		/*Initial configuration of bmi160*/
		uint8_t data[2] = {WM8731_CODEC_POWER_DOWN, 0x00};
		/*Acc config*/
		wm8731_sucess = freertos_i2c_send_receive(WM8731_CODEC_cfg.i2c_number, data , 2, WM8731_SLAVE_ADDRESS,0,0,i2c_write);
		vTaskDelay(pdMS_TO_TICKS(I2C_DELAY));

		data[0] = WM8731_CODEC_LEFT_IN;
		data[1] = 23;
		wm8731_sucess = freertos_i2c_send_receive(WM8731_CODEC_cfg.i2c_number, data , 2, WM8731_SLAVE_ADDRESS,0,0,i2c_write);
		vTaskDelay(pdMS_TO_TICKS(I2C_DELAY));

		data[0] = WM8731_CODEC_RIGHT_IN;
		data[1] = 23;
		wm8731_sucess = freertos_i2c_send_receive(WM8731_CODEC_cfg.i2c_number, data , 2, WM8731_SLAVE_ADDRESS,0,0,i2c_write);
		vTaskDelay(pdMS_TO_TICKS(I2C_DELAY));


		data[0] = WM8731_CODEC_ANALOG;
		data[1] = 0x05;
		wm8731_sucess = freertos_i2c_send_receive(WM8731_CODEC_cfg.i2c_number, data , 2, WM8731_SLAVE_ADDRESS,0,0,i2c_write);
		vTaskDelay(pdMS_TO_TICKS(I2C_DELAY));

		data[0] = WM8731_CODEC_CODEC_ENABLE;
		data[1] = 0x01;
		wm8731_sucess = freertos_i2c_send_receive(WM8731_CODEC_cfg.i2c_number, data , 2, WM8731_SLAVE_ADDRESS,0,0,i2c_write);
		vTaskDelay(pdMS_TO_TICKS(I2C_DELAY));


	}
	return wm8731_sucess;
}
