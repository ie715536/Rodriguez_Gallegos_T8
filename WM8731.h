/*
 * WM8731.h
 *
 *  Created on: 13 nov. 2020
 *      Author: urik_
 */

#ifndef WM8731_H_
#define WM8731_H_

#include "I2C_FreeRtos.h"
#include "fsl_clock.h"
#include "fsl_port.h"
#include "stdint.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

typedef enum
{
	freertos_WM8731_sucess,
	freertos_WM8731_fail
} freertos_WM8731_flag_t;

#endif /* WM8731_H_ */
