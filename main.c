#include <stdio.h>
#include <wm8731_CODEC.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"
/* TODO: insert other include files here. */
#include "i2c_rtos.h"
#include "task.h"
#include "semphr.h"
/* TODO: insert other definitions and declarations here. */
/*
 * @brief   Application entry point.
 */
SemaphoreHandle_t i2c_sem;

void init_codec(void *parameters)
{
	uint8_t g_codec_sucess = freertos_i2c_fail;
	g_codec_sucess = wm8731_init();
	if(freertos_i2c_sucess == g_codec_sucess)
	{
		PRINTF("Codec configured\n\r");
	}
	xSemaphoreGive(i2c_sem);
	vTaskSuspend(NULL);
}

void codec_audio(void *parameters)
{

	xSemaphoreTake(i2c_sem, portMAX_DELAY);

	for(;;)
	{
		/*TODO AUDIO FUNCTIONS*/
		vTaskDelay(pdMS_TO_TICKS(300));
	}
}

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

    xTaskCreate(init_codec, "init_codec", 110, NULL, 1, NULL);
    xTaskCreate(codec_audio, "codec_audio", 110, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1)
    {

    }
    return 0 ;
}
