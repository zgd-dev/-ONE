#include "esp_at.h"
#include "command.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include "stdlib.h"
#include "libemqtt.h"
#include "pal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "flash.h"
extern uint8_t a[20];


//// ==================== MQTT 初始化（WiFi + TCP + 登录 + 订阅   成功后显示设备在线） ====================
/*MCU向ESP8266发送AT指令 接收后返回应答给MCU MCU将其写入缓冲区 接着从缓冲区里查找期望应答*/
void MQTT_Init(void)
{

  ESP_SendCmd_OK("AT+CWMODE=1\r\n", 2000);
	ESP_SendCmd("AT+CWJAP=\"aaa\",\"18208901719\"\r\n", "WIFI GOT IP", 5000);



	//	  ESP_SendCmd_OK("AT+CIPMUX=0\r\n", 2000);
	/*tcp连接*/
	ESP_SendCmd("AT+CIPSTART=\"TCP\",\"bemfa.com\",9501\r\n", "CONNECT", 5000);


  // 先退出透传模式 再重新进入透传
	HAL_UART_Transmit(&huart1, (uint8_t *)"+++", 3, 100);
  vTaskDelay(1000);
	ESP_SendCmd("AT+CIPCLOSE\r\n", "OK", 2000);
  vTaskDelay(200);

	


  ESP_SendCmd_OK("AT+CIPMODE=1\r\n", 2000);								//启动透传
  HAL_UART_Transmit(&huart1, (uint8_t *)"AT+CIPSEND\r\n", sizeof("AT+CIPSEND\r\n")-1, 1000);			//进入透传发送
  ESP_WaitFor(">", 2000);
	vTaskDelay(500);  // ✅ 必须 ≥500ms
	RingBuf_Clear();	
  // MQTT 登录
  uint8_t connect_packet[] = {
    0x10, 0x2E,
    0x00,0x06,
    0x4D,0x51,0x49,0x73,0x64,0x70,
    0x03, 0x02, 0x00,0x78,						 //  👉 心跳保活时间 = 120 秒（关键） 超过120秒要主动向服务器发心跳包
    0x00,0x20,
    '4','1','f','3','8','7','6','7',
    '2','0','a','d','4','a','e','8',
    '6','0','d','b','0','6','e','3',
    '5','6','3','1','a','8','5','5'
  };
  HAL_UART_Transmit(&huart1, connect_packet, sizeof(connect_packet), 5000);
  vTaskDelay(300);
	
	
    // ====== 接收 CONNACK  确认连接真的成功了 ======
    uint8_t connack[4];
		int ret = pal_tcp_recv_raw(0, connack, 4, 3000);								//读缓冲区 返回长度
			if (ret == 4 && connack[0] == 0x20 && connack[1] == 0x02 && connack[3] == 0x00)
				{     
					g_mqtt_connected = 1;
					  // 订阅主题
					uint8_t sub_packet[] = {
						0x82, 0x0A,
						0x00, 0x01,
						0x00, 0x05,
						's','t','m','3','2',
						0x01
																	};
					HAL_UART_Transmit(&huart1, sub_packet, sizeof(sub_packet), 5000);
					vTaskDelay(300);
				} else {
        g_mqtt_connected = 0;
			
    }
		


}



// ==================== MQTT 心跳包 ====================
void MQTT_SendPing(void)
{
  uint8_t ping[] = {0xC0, 0x00};
  HAL_UART_Transmit(&huart1, ping, 2, 2000);
}

/**
 * @brief  透传模式下从 ESP8266 接收原始 TCP 数据
 * @param  sock       未使用（保留参数，兼容之前的接口设计）
 * @param  buf        接收缓冲区指针
 * @param  len        期望接收的字节数
 * @param  timeout_ms 超时时间（毫秒）
 * @return 实际接收到的字节数，超时或失败返回 -1
 */

int pal_tcp_recv_raw(int sock, uint8_t *buf, int len, int timeout_ms)
{
    uint32_t start = HAL_GetTick();
    int received = 0;
    while (received < len) {
        if (HAL_GetTick() - start > timeout_ms) {
            return -1;
        }
        uint8_t ch;
        while (RingBuf_Read(&ch) && received < len) {
            buf[received++] = ch;
        }
        //HAL_Delay(1); // 避免CPU空转
				vTaskDelay(1);
    }
    return received;
}

// 在 mqtt.c 中新增
void MQTT_SendTempHumi_NoWait(float temp, float humi)
{
    char payload[128];
    sprintf(payload, "{\"temperature\":%.1f,\"shidu\":%.1f}", temp, humi);

    uint8_t buf[128];
    int pos = 0;
    buf[pos++] = 0x32;		//固定报头 报文类型

		/*可变报头 主题长度：2 主题名称：5 报文标识符：2 有效载荷：*/
    uint16_t topic_len = 5;
    uint16_t msg_len = strlen(payload);
    uint16_t rem = 2 + topic_len + 2 + msg_len;

		/*变长编码计算剩余长度 小端排序（低位数据放在低地址） 低位在前  (一般低地址在前)*/
    do {
        uint8_t b = rem % 128;
        rem /= 128;
        if (rem > 0) b |= 0x80;
        buf[pos++] = b;
    } while (rem > 0);

		/*可变报头  主题长度 0x00 0x05两个字节 高位在前*/
    buf[pos++] = 0;
    buf[pos++] = topic_len;
    memcpy(&buf[pos], "stm32", 5);
    pos += 5;

		/*报文标识符 0x00 0x01*/
    buf[pos++] = 0;
    buf[pos++] = 1;

		/*有效载荷*/
    memcpy(&buf[pos], payload, msg_len);
    pos += msg_len;

    HAL_UART_Transmit(&huart1, buf, pos, 5000);
    vTaskDelay(100);
    // 不等待 PUBACK，直接返回
}