/***************************************************************************//**
 * @file
 * @brief FreeRTOS Blink Demo for Energy Micro EFM32GG_STK3700 Starter Kit
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "croutine.h"

#include "em_chip.h"
#include "bsp.h"
#include "bsp_trace.h"

#include "sleep.h"

#include "myLibI2C.h"

#define STACK_SIZE_FOR_TASK    (configMINIMAL_STACK_SIZE + 10)
#define TASK_PRIORITY          (tskIDLE_PRIORITY + 1)

/* Structure with parameters for LedBlink */
typedef struct {
  /* Delay between blink of led */
  portTickType delay;
  /* Number of led */
  int          ledNo;
} TaskParams_t;

// Init I2C Struct
typedef struct {
  uint8_t address;
  uint8_t reg;
} InitI2CParameters;

// Init Proximity Sensor Struct
typedef struct {
  uint8_t reg_1; // 0x80
  uint8_t reg_2; // 0x89
  uint8_t reg_3; // 0x8B
  uint8_t reg_4; // 0x8C
} InitProximitySensorParameters;

typedef struct {
  uint8_t reg_1;
  uint8_t reg_2;
  uint8_t reg_3;
} ScanProximitySensorParameters;

/***************************************************************************//**
 * @brief Simple task which is blinking led
 * @param *pParameters pointer to parameters passed to the function
 ******************************************************************************/
static void LedBlink(void *pParameters)
{
  TaskParams_t     * pData = (TaskParams_t*) pParameters;
  const portTickType delay = pData->delay;

  for (;; ) {
    BSP_LedToggle(pData->ledNo);
    vTaskDelay(delay);
  }
}

static void InitI2C(void *pParameters)
{
  InitI2CParameters * pdata = (InitI2CParameters*) pParameters;
  uint8_t reg = pdata->reg;
  uint8_t address = pdata->address;
  address = (address << 1);
  uint8_t value;
  init(address);
  sensorReadRegister(reg, &value);
}

static void InitProximitySensor(void *pParameters)
{
	InitProximitySensorParameters * pdata = (InitProximitySensorParameters*) pParameters;

	uint8_t reg1 = pdata->reg_1;
  uint8_t reg2 = pdata->reg_2;
  uint8_t reg3 = pdata->reg_3;
  uint8_t reg4 = pdata->reg_4;
  // Turn Interrupts ON | ON Sensor
  uint8_t data1 =  /*(1 << 5) |*/ 1;
  // SET MIN THRESHOLD
  uint8_t data2 = 96;
  // SET MAX THRESHOLD
  uint8_t data3 = 144;
  // SET REPETITIONS TO 1 (PPERS)
  uint8_t data4 = 1;
  // Write on Registers
  sensorWriteRegister(reg1, data1);
  sensorWriteRegister(reg1, data2);
  sensorWriteRegister(reg1, data3);
  sensorWriteRegister(reg1, data4);

}

static void ScanProximitySensor(void *pParameters)
{
	ScanProximitySensorParameters * pdata = (ScanProximitySensorParameters*) pParameters;

	uint8_t reg1 = pdata->reg_1; //0x80
	uint8_t reg2 = pdata->reg_2; // 0x93 - PVALID
	uint8_t reg3 = pdata->reg_3; // 0x9C - PDATA
	uint8_t data1 =  (1 << 2);

	// VALUES WE READ
	uint8_t value2;
	uint8_t value3;

	// Turn Proximity Sensor ON

	sensorWriteRegister(reg1, data1);
	while(1)
	{
		// READING PVALID
		sensorReadRegister(reg2, &value2);
		if (value2 & 1)
			break;
	}
	// READING PDATA
	sensorReadRegister(reg3, &value3);
}
/***************************************************************************//**
 * @brief  Main function
 ******************************************************************************/
int main(void)
{
  /* Chip errata */
  CHIP_Init();
  /* If first word of user data page is non-zero, enable Energy Profiler trace */
  BSP_TraceProfilerSetup();

  /* Initialize LED driver */
  BSP_LedsInit();
  /* Setting state of leds*/
  BSP_LedSet(0);
  BSP_LedSet(1);

  /* Initialize SLEEP driver, no calbacks are used */
  SLEEP_Init(NULL, NULL);
#if (configSLEEP_MODE < 3)
  /* do not let to sleep deeper than define */
  SLEEP_SleepBlockBegin((SLEEP_EnergyMode_t)(configSLEEP_MODE + 1));
#endif


  static InitI2CParameters paramsInitI2C = {0x39, 0x92};
  static InitProximitySensorParameters paramsInitProximitySensor = {0x80, 0x89, 0x8B, 0x8C};
  static ScanProximitySensorParameters paramsScanProximitySensor = {0x80, 0x93, 0x9C};

  /*Create Prova la Pulga task*/
  xTaskCreate(InitI2C, (const char *) "InitI2C", STACK_SIZE_FOR_TASK, &paramsInitI2C, TASK_PRIORITY, NULL);
  // INIT TASK
  xTaskCreate(InitProximitySensor, (const char *) "InitProximitySensor", STACK_SIZE_FOR_TASK, &paramsInitProximitySensor, TASK_PRIORITY, NULL);
  // READ TASK
  xTaskCreate(ScanProximitySensor, (const char *) "InitProximitySensor", STACK_SIZE_FOR_TASK, &paramsScanProximitySensor, TASK_PRIORITY, NULL);

  /* Parameters value for taks*/
    static TaskParams_t parametersToTask1 = { pdMS_TO_TICKS(1000), 0 };
    static TaskParams_t parametersToTask2 = { pdMS_TO_TICKS(500), 1 };


  /*Create two task for blinking leds*/
  xTaskCreate(LedBlink, (const char *) "LedBlink1", STACK_SIZE_FOR_TASK, &parametersToTask1, TASK_PRIORITY, NULL);
  xTaskCreate(LedBlink, (const char *) "LedBlink2", STACK_SIZE_FOR_TASK, &parametersToTask2, TASK_PRIORITY, NULL);



  /*Start FreeRTOS Scheduler*/
  vTaskStartScheduler();

  return 0;
}
