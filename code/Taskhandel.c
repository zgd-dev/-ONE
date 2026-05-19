#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cmsis_os.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "lcd.h"
#include "touch.h"
#include "stdio.h"
#include "string.h"
#include "lvgl.h"               
#include "lv_port_disp.h"       
#include "lv_port_indev.h"      
#include "page_main.h"
#include "modbus.h"
#include "command.h"
#include "ESP_at.h"
#include "libemqtt.h"
#include "pal.h"
#include "Taskhandel.h"
#include "bsp_flash.h"
#include "flash.h"

 uint8_t fail_count = 0; // 记录连续失败次数
QueueHandle_t     data_queue;
SemaphoreHandle_t uart1_mutex;

QueueHandle_t lvgl_data_queue;   
uint32_t last_ping = 0;

 volatile uint8_t g_mqtt_connected;
// ==============================================
// Modbus 采集
// ==============================================
void Modbus(void)
{
    Data_t data;				//结构体变量

    while(1)
    {
        Modbus_Get_Temp(&data.temp, &data.shi);		//采集温湿度            
				xQueueOverwrite(data_queue, &data);					// 永远只保留最新一次采集结果，旧值被直接覆盖 该队列长度为1
        vTaskDelay(1000);
    }
}


/*补传绿灯亮 正常红灯亮 离线存储黄灯亮*/
/*
正常在线：先补传历史数据，再发送实时数据，通知 UI 更新。

断网期间：把数据存入 SPI Flash，等待恢复。

重连后：自动补传缓存数据（每次最多 5 条，分批补传）。

*/
void MQTT(void)
{
    vTaskDelay(5000);
    Data_t data_q;
    while (1)
    {
				 xSemaphoreTake(uart1_mutex, portMAX_DELAY);				//获取锁
        if (xQueueReceive(data_queue, &data_q, pdMS_TO_TICKS(3000)) == pdPASS)			//接收队列 等待3秒modbus写队列
        {
//            xSemaphoreTake(uart1_mutex, portMAX_DELAY);				//获取锁

            if (g_mqtt_connected)			//在线
            {
                char cached[512];
                uint16_t len;
                int count = 0;

                // 补传（读完就销毁，绝对不重复）
							while (FlashCache_HasData() && count < 5)					//如果读指针落后写指针，说明有数据 返回1
							{
								if (FlashCache_Read(cached, &len) == 0)					//读缓冲区数据
									{
											HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);
											HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);
											MQTT_PublishJson(cached);
											FlashCache_MarkSent();				//只改魔数，不擦除、改魔数标记已补传
											count++;
											vTaskDelay(800);
									}
									else
									{
										
											my_clear();		//补传五次数据完成后 清空缓存 让读地址等于写地址
											break;
									}
							}

                // 发实时数据
                MQTT_SendTempHumi_NoWait(data_q.temp, data_q.shi);
								HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET);               
								HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);							
                xQueueOverwrite(lvgl_data_queue, &data_q);			// 写队列 永远只保留最新一次采集结果，旧值被直接覆盖 该队列长度为1
            }
            else	//断线中
            {
                // 离线缓存
                char json[128];
                sprintf(json, "{\"temperature\":%.1f,\"shidu\":%.1f}", data_q.temp, data_q.shi);
                FlashCache_Write(json, strlen(json));
            }

            xSemaphoreGive(uart1_mutex);					//释放锁
        }
        vTaskDelay(2000);
    }
}



void AT(void)
{
    MQTT_Init();

    uint8_t heart_fail = 0;  // 心跳连续失败次数

    for (;;)
    {
        vTaskDelay(10000);  // 每 10 秒检测一次
																
        xSemaphoreTake(uart1_mutex, portMAX_DELAY);	//获取互斥锁
				RingBuf_Clear();														//关键点 先清空缓冲区 否则有脏数据影响判断
        if (g_mqtt_connected)
        {
            // 发送 PINGREQ
            MQTT_SendPing();

            uint8_t pong[2];
            int ret = pal_tcp_recv_raw(0, pong, 2, pdMS_TO_TICKS(1000));			// 等待 PINGRESP（1 秒超时）

            if (ret>=2 && pong[0] == 0xD0 && pong[1] == 0x00)
            {
                // 心跳正常
                heart_fail = 0;
            }
            else
            {
                // 心跳超时或数据错误
                heart_fail++;
                if (heart_fail >= 3)  // 连续 3 次失败，约 30 秒
                {
                    g_mqtt_connected = 0;					//断网
									
										HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
										HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);
                    heart_fail = 0;
                }
            }
        }



        xSemaphoreGive(uart1_mutex);					//释放锁
    }
}
// ==============================================
// UI 显示
// ==============================================
void UI(void)
{
    Data_t disp_data;

    LCD_Init();
    TP_Init();
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    lv_obj_t *main_scr = page_main_create();		//屏幕界面
    lv_scr_load(main_scr);				//加载界面

    uint32_t start_tick = xTaskGetTickCount();
    
    uint8_t  last_status = 0xFF;      // 记录上一次的 MQTT 状态
    uint32_t last_uptime  = 0xFFFFFFFF; // 记录上一次的运行时间    魔法数

    while (1)
    {
        // 更新温湿度（只有新数据到来时才更新）
        if (xQueueReceive(lvgl_data_queue, &disp_data, 0) == pdPASS)
        {
            page_main_update_data(disp_data.temp, disp_data.shi);
        }

        // 只在 MQTT 状态发生变化时才更新显示
        if (g_mqtt_connected != last_status)
        {
            last_status = g_mqtt_connected;
            page_main_update_status(g_mqtt_connected);
        }

        // 只在运行时间发生变化时才更新显示（每秒变一次）
        uint32_t uptime = (xTaskGetTickCount() - start_tick) / configTICK_RATE_HZ;
        if (uptime != last_uptime)
        {
            last_uptime = uptime;					//虽然说是局部变量 但是这是任务并没有退出该函数 所以局部变量并没有被清空回收
            page_main_update_uptime(uptime);
        }


        lv_timer_handler();
        vTaskDelay(5);
    }
}



