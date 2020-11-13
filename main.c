/**
 * @file    freertos_uart.c
 * @brief   Application entry point.
 */

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK66F18.h"
#include "fsl_debug_console.h"
#include "FreeRTOSConfig.h"
/* TODO: insert other include files here. */
#include "FreeRTOS.h"
#include "task.h"


typedef struct{
	uint32_t header;
	float x;
	float y;
	float z;
}comm_msg_t;

/* TODO: insert other definitions and declarations here. */
#define MSSG_HEADER 0xAAAAAAAA

void init_BMI160_task(void * args);
void init_UART_task(void * args);
void get_and_send_data_task(void * args);

SemaphoreHandle_t BMI160_initialization;
SemaphoreHandle_t UART_initialization;

int main(void)
{

  /* Init board hardware. */
  BOARD_InitBootPins();
  BOARD_InitBootClocks();
  BOARD_InitBootPeripherals();
  /* Init FSL debug console. */
  BOARD_InitDebugConsole();

  xTaskCreate(init_BMI160_task, "init_BMI160", 110, NULL, 1, NULL);
  xTaskCreate(init_UART_task, "init_UART", 110, NULL, 1, NULL);
  xTaskCreate(get_and_send_data_task, "get_data", 110, NULL, 1, NULL);

  BMI160_initialization = xSemaphoreCreateBinary();
  UART_initialization = xSemaphoreCreateBinary();

  vTaskStartScheduler();

  volatile static int i = 0 ;

  while(1)
  {
    i++;
    __asm volatile ("nop");
  }

  return 0 ;
}

void init_BMI160_task(void * args)
{
	freertos_bmi160_flag_t status = freertos_bmi160_fail;
	status = BMI160_init();
	if(freertos_bmi160_sucess != status)
	{
		PRINTF("FAIL INITIALIZATION");
		for(;;);
	}
	xSemaphoreGive(BMI160_initialization);
	vTaskDelay(portMAX_DELAY); /** Delay infinito */
}

void init_UART_task(void * args)
{
	freertos_uart_config_t config;

	config.baudrate = 115200;
	config.rx_pin = 16;
	config.tx_pin = 17;
	config.pin_mux = kPORT_MuxAlt3;
	config.uart_number = freertos_uart0;
	config.port = freertos_uart_portB;
	freertos_uart_init(config);

	xSemaphoreGive(UART_initialization);
	vTaskDelay(portMAX_DELAY); /** Delay infinito */
}

void get_and_send_data_task(void * args)
{
	bmi160_raw_data_t acc_data;
	bmi160_raw_data_t gyro_data;
	TickType_t xfactor = pdMS_TO_TICKS(20);

	xSemaphoreTake(BMI160_initialization, portMAX_DELAY);
	vTaskDelay( pdMS_TO_TICKS(1000));
	xSemaphoreTake(UART_initialization, portMAX_DELAY);

	TickType_t  xLastWakeTime = xTaskGetTickCount();

	while(true)
	{
		vTaskDelayUntil(&xLastWakeTime,xfactor);
		MahonyAHRSEuler_t angles;
		comm_msg_t message;
		acc_data = BMI160_get_ACCEL();
		gyro_data = BMI160_get_GYRO();

		//PRINTF("Accel \tx: %d\ty: %d\tz: %d\t\r\n", acc_data.x, acc_data.y, acc_data.z);
		//PRINTF("Giro: \tx: %d\ty: %d\tz: %d\t\r\n", gyro_data.x, gyro_data.y, gyro_data.z);

		angles = MahonyAHRSupdateIMU((float)gyro_data.x, (float)gyro_data.y, (float)gyro_data.z,(float)acc_data.x, (float)acc_data.y, (float)acc_data.z);
		message.header = MSSG_HEADER;
		message.x = angles.roll;
		message.y = angles.pitch;
		message.z = angles.yaw;


		freertos_uart_send(freertos_uart0, (uint8_t *)&message, sizeof(comm_msg_t));

		//vTaskDelay( pdMS_TO_TICKS(1000));
	}
}


