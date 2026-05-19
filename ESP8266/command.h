#ifndef COMMAND_H
#define COMMAND_H

#include "main.h"
#include <string.h>
#include "usart.h"
#include "stm32f4xx.h"
#include "stdio.h"

#define BUFFER_SIZE 128
#define COMMAND_MIN_LENGTH 4
uint8_t Command_Write(uint8_t *data, uint8_t length);

uint8_t Command_GetCommand(uint8_t *command);
uint8_t RingBuf_Read(uint8_t *out) ;
void RingBuf_Clear(void);
#endif /* INC_COMMAND_H_ */
