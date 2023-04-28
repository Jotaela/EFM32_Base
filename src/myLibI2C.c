#include "myLibI2C.h"
#include "teacherLibI2C.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "projdefs.h"

SemaphoreHandle_t semphore;

int init(uint8_t address) {
	semphore = xSemaphoreCreateBinary();
	xSemaphoreGive(semphore);
	if (semphore == NULL) {
		return 1;
	} else {
		BSP_I2C_Init(address);
		return 0;
	}
}


int sensorWriteRegister(uint8_t reg, uint8_t data) {
	if( xSemaphoreTake( semphore,portMAX_DELAY ) == pdTRUE )
	{
		I2C_WriteRegister(reg, data);
		if (xSemaphoreGive(semphore) != pdTRUE)
			return 1;
		return 0;
	}
	return 1;
}

int sensorReadRegister(uint8_t reg, uint8_t *val) {
	if( xSemaphoreTake( semphore, portMAX_DELAY ) == pdTRUE )
	{
		I2C_ReadRegister(reg, val);
		if (xSemaphoreGive(semphore) != pdTRUE)
			return 1;
		return 0;
	}
	return 1;
}
