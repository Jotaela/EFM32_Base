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

typedef struct {
  uint8_t address;
  uint8_t reg;
} TaskPulga_t;

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

static void ProvaLaPulga(void *pParameters)
{
  TaskPulga_t * pdata = (TaskPulga_t*) pParameters;
  uint8_t reg = pdata->reg;
  uint8_t address = pdata->address;
  address = (address << 1);
  uint8_t value;
  init(address);
  sensorReadRegister(reg, &value);
  printf("El resultat és: %d \n", value);
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


  static TaskPulga_t paramsPulga = {0x39, 0x92};

  /*Create Prova la Pulga task*/
  xTaskCreate(ProvaLaPulga, (const char *) "LaPulga", STACK_SIZE_FOR_TASK, &paramsPulga, TASK_PRIORITY, NULL);

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
