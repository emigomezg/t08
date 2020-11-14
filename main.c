#include <stdio.h>
#include <wm8731_CODEC.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"
#include "i2c_rtos.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t i2c_sem;

void wm8731_play(void *parameters);
void wm8731_CODEC_init(void *parameters);

int main(void)
{
  	/* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif

    i2c_sem = xSemaphoreCreateBinary();

    xTaskCreate(wm8731_CODEC_init, "wm8731_CODEC_init", 110, NULL, 1, NULL);
    xTaskCreate(wm8731_play, "wm8731_play", 110, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1)
    {

    }
    return 0 ;
}

void wm8731_play(void *parameters)
{
	xSemaphoreTake(i2c_sem, portMAX_DELAY);
	for(;;)
	{
		vTaskDelay(pdMS_TO_TICKS(400));
	}
}

void wm8731_CODEC_init(void *parameters)
{
	uint8_t g_codec_sucess = freertos_i2c_fail;
	g_codec_sucess = wm8731_init();
while(!g_codec_sucess){}
	xSemaphoreGive(i2c_sem);
	vTaskSuspend(NULL);
}
