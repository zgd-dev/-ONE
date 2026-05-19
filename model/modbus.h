#ifndef MODBUS_H
#define MODBUS_H
#include "stdint.h"
#include "stm32f4xx.h"

#define RS485_TX  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET)
#define RS485_RX  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET)


int Modbus_Get_Temp(float *temp,float *shi);

uint16_t CRC_16_MODBUS(uint8_t *data,uint8_t len,uint8_t flag);




#endif
