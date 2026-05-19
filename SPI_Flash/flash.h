#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

void FlashCache_Init(void);
int  FlashCache_Write(const char *json, uint16_t len);
int  FlashCache_Read(char *json, uint16_t *len);
int  FlashCache_HasData(void);
void FlashCache_Clear(void);
void MQTT_PublishJson(const char *json);
void MQTT_SendTempHumi_WithCache(float temp, float humi);
extern volatile uint8_t g_mqtt_connected;
void FlashCache_MarkSent(void);
int FlashCache_ReadAndConsume(char *json, uint16_t *len);
void my_clear();
#endif

