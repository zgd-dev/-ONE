#ifndef __ESP_AT_H__
#define __ESP_AT_H__

#include <stdint.h>

typedef enum {
    ESP_OK      = 0,
    ESP_ERROR   = 1,
    ESP_TIMEOUT = 2
} ESP_Status_t;

// 通用 AT 指令发送（带超时和期望应答）
ESP_Status_t ESP_SendCmd(const char *cmd, const char *expected_ack, uint32_t timeout_ms);

// 简化版：发指令等 "OK"
ESP_Status_t ESP_SendCmd_OK(const char *cmd, uint32_t timeout_ms);
int ESP_WaitFor(const char *keyword, uint32_t timeout_ms);

#endif