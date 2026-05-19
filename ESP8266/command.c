#include "command.h"
#include "usart.h"
#include "stm32f4xx.h"
#include "dma.h"
// 指令的最小长度

extern DMA_HandleTypeDef hdma_usart1_rx;
// 循环缓冲区大小

// 循环缓冲区
uint8_t buffer[BUFFER_SIZE];
// 循环缓冲区读索引
uint8_t readIndex = 0;
// 循环缓冲区写索引
uint8_t writeIndex = 0;

/**
 * @brief  清空环形缓冲区（重置读写指针）
 */
void RingBuf_Clear(void)
{
    readIndex = 0;
    writeIndex = 0;
}
extern uint8_t a[20];
/**
* @brief 增加读索引
* @param length 要增加的长度
*/
void Command_AddReadIndex(uint8_t length) {
    readIndex += length;
    readIndex %= BUFFER_SIZE;
}

/**
* @brief 读取第i位数据 超过缓存区长度自动循环
* @param i 要读取的数据索引
*/

uint8_t Command_Read(uint8_t i) {
    uint8_t index = i % BUFFER_SIZE;
    return buffer[index];
}

/*获取read下标对应的数据*/
uint8_t RingBuf_Read(uint8_t *out) {
    if (readIndex == writeIndex) {
        return 0;   // 缓冲区空
    }
    *out = buffer[readIndex];
    readIndex = (readIndex + 1) % BUFFER_SIZE;
    return 1;
}


uint8_t Command_GetLength() {
    return (writeIndex + BUFFER_SIZE - readIndex) % BUFFER_SIZE;
}


/**
* @brief 计算缓冲区剩余空间
* @return 剩余空间
* @retval 0 缓冲区已满
* @retval 1~BUFFER_SIZE-1 剩余空间
* @retval BUFFER_SIZE 缓冲区为空
*/
uint8_t Command_GetRemain() {
    return BUFFER_SIZE - Command_GetLength();
}

/**
* @brief 向缓冲区写入数据
* @param data 要写入的数据指针
* @param length 要写入的数据长度
* @return 写入的数据长度
*/
uint8_t Command_Write(uint8_t *data, uint8_t length) {
    // 如果缓冲区不足 则不写入数据 返回0
    if (Command_GetRemain() < length) {
        return 0;
    }
    // 使用memcpy函数将数据写入缓冲区
    if (writeIndex + length < BUFFER_SIZE) {
        memcpy(buffer + writeIndex, data, length);
        writeIndex += length;
    } else {
        uint8_t firstLength = BUFFER_SIZE - writeIndex;
        memcpy(buffer + writeIndex, data, firstLength);
        memcpy(buffer, data + firstLength, length - firstLength);
        writeIndex = length - firstLength;
    }
    return length;
}


/**
* @brief 从缓冲区读取所有未处理的数据（不解析指令帧）
* @param command 存储读取数据的指针
* @return 实际读取的字节数（0 表示缓冲区为空）
*/
uint8_t Command_GetCommand(uint8_t *command) {
    uint8_t len = Command_GetLength();  // 获取当前未处理的数据长度
    if (len == 0) {
        return 0;
    }
    // 顺序读取 len 个字节到 command 中
    for (uint8_t i = 0; i < len; i++) {
        command[i] = Command_Read(readIndex + i);
    }
    Command_AddReadIndex(len);  // 移动读索引，表示这些数据已被消费
    return len;
}

/*接收完成回调*/
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart==&huart1)
	{

		Command_Write(a,Size);		//向缓冲区写入数据
		
		HAL_UARTEx_ReceiveToIdle_DMA(&huart1,a,sizeof(a));
		__HAL_DMA_DISABLE_IT(&hdma_usart1_rx,DMA_IT_HT);
	}
}