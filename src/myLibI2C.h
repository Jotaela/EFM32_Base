#include <stdint.h>

int init(uint8_t address);
int sensorWriteRegister(uint8_t reg, uint8_t data);
int sensorReadRegister(uint8_t reg, uint8_t *val);

