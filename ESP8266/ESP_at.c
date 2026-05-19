#include "esp_at.h"
#include "command.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

// 引用阶段一创建的全局环形缓冲区
extern uint8_t buffer[BUFFER_SIZE];

// 清空环形缓冲区（丢弃所有旧数据）
static void ESP_ClearRxBuf(void) {
    uint8_t dummy;
    while (RingBuf_Read(&dummy));
}



/*
 * 通用 AT 指令发送并逐行解析应答       
 * cmd           : 要发送的 AT 指令（必须包含 \r\n）
 * expected_ack  : 期望出现的应答关键词，如 "OK", "CONNECT", "WIFI GOT IP"
 * timeout_ms    : 超时时间（毫秒）
 * 返回         : ESP_OK(成功), ESP_TIMEOUT(超时), ESP_ERROR(未使用但预留)
 */
//发送 AT 指令 →ESP8266返回数据-> 从环形缓冲区逐行读取应答 → 匹配期望关键词 → 返回成功或超时。
ESP_Status_t ESP_SendCmd(const char *cmd, const char *expected_ack, uint32_t timeout_ms)
{
    // 1. 清空环形缓冲区，丢弃旧数据 逻辑清空 移动读位
    ESP_ClearRxBuf();

    // 2. 发送指令
    HAL_UART_Transmit(&huart1, (uint8_t *)cmd, strlen(cmd), 1000);

    // 3. 在超时时间内逐字节读取，按行解析
    char line[512];          // 存放一行数据
    uint16_t line_pos = 0;   // 当前行写入位置
    uint32_t start_tick = HAL_GetTick();

    while ((HAL_GetTick() - start_tick) < timeout_ms) {
        uint8_t ch;
			while (RingBuf_Read(&ch)) {									//读缓冲区
            start_tick = HAL_GetTick();					 // 关键：每收到一个字节，就重置超时起点

            if (ch == '\n') {
                // 一行结束
                line[line_pos] = '\0';
                
                // 检查是否包含期望应答
                if (expected_ack != NULL && strstr(line, expected_ack) != NULL) {
                    return ESP_OK;
                }
                
                // 检查是否出错
                if (strstr(line, "ERROR") != NULL) {
                    return ESP_ERROR;
                }
                
                // 清空 line，准备读下一行
                line_pos = 0;
            } 
            else if (ch != '\r') {
                // 非回车字符，追加到 line
                if (line_pos < sizeof(line) - 1) {
                    line[line_pos++] = ch;
                }
            }
        }
    }
    
    return ESP_TIMEOUT;
}

// 简化版：发指令，只等待 "OK"
ESP_Status_t ESP_SendCmd_OK(const char *cmd, uint32_t timeout_ms)
{
    return ESP_SendCmd(cmd, "OK", timeout_ms);
}




/**
 * @brief  等待环形缓冲区中出现指定关键字（超时返回）
 * @param  keyword   要匹配的关键字（字符串）
 * @param  timeout_ms 超时时间（毫秒）
 * @return 1-匹配成功，0-超时未匹配
 */
int ESP_WaitFor(const char *keyword, uint32_t timeout_ms)
{
    int kw_len = strlen(keyword);
    if (kw_len == 0) return 1;  // 空字符串视为立即成功

    char match_buf[kw_len];     // 滑动窗口，保存最近 kw_len 个字节
    memset(match_buf, 0, kw_len);
    int pos = 0;                // 累计已读取字节数
    uint32_t start = HAL_GetTick();

    while ((HAL_GetTick() - start) < timeout_ms)
    {
        uint8_t ch;
        if (RingBuf_Read(&ch))
        {
            // 将新字节写入环形窗口
            match_buf[pos % kw_len] = ch;
            pos++;

            // 只有填满窗口后才开始匹配
            if (pos >= kw_len)
            {
                // 窗口中最旧字节的索引（即最近 kw_len 个字节的起始位置）
                int start_idx = (pos - kw_len) % kw_len;

                // 数据在 match_buf 中可能分成两段：
                // 第一段：从 start_idx 到数组末尾
                // 第二段：从 0 到 start_idx-1
                int first_len = kw_len - start_idx;       // 第一段长度
                int second_len = kw_len - first_len;      // 第二段长度，即 start_idx

                // 比较第一段（如果 first_len > 0）
                if (first_len > 0 &&
                    memcmp(&match_buf[start_idx], keyword, first_len) != 0)
                {
                    continue;   // 第一段不匹配，直接继续
                }

                // 比较第二段（如果存在）
                if (second_len > 0 &&
                    memcmp(&match_buf[0], keyword + first_len, second_len) != 0)
                {
                    continue;
                }

                // 两段都匹配成功
                return 1;
            }
        }
        // 可以适当加一个短延时避免忙等待，但视系统而定，此处省略
    }
    return 0;   // 超时
}