#include "flash.h"
#include "bsp_flash.h"
#include <string.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

#define CACHE_START_ADDR    (4 * 1024 * 1024)
#define CACHE_END_ADDR      (8 * 1024 * 1024)
#define CACHE_SECTOR_SIZE   4096
#define CACHE_ENTRY_SIZE    128

#define MAGIC_VALID         0xDEADBEEF
#define MAGIC_EMPTY         0xFFFFFFFF

extern UART_HandleTypeDef huart1;
typedef struct {
    uint32_t magic;
    uint32_t timestamp;
    uint16_t data_len;
    uint8_t  data[CACHE_ENTRY_SIZE - 12];
} CacheEntry_t;

static uint32_t write_addr = CACHE_START_ADDR;
static uint32_t read_addr  = CACHE_START_ADDR;
static uint8_t  initialized = 0;

// 初始化
void FlashCache_Init(void)
{
    CacheEntry_t entry;
    uint32_t addr = CACHE_START_ADDR;

    write_addr = CACHE_START_ADDR;
    while (addr < CACHE_END_ADDR) {
        SPI_FLASH_BufferRead((uint8_t *)&entry, addr, sizeof(CacheEntry_t));
        if (entry.magic == MAGIC_VALID) {
            write_addr = addr + sizeof(CacheEntry_t);
        } else {
            break;
        }
        addr += sizeof(CacheEntry_t);
    }

    if (write_addr >= CACHE_END_ADDR) {
        write_addr = CACHE_START_ADDR;
    }

    read_addr = CACHE_START_ADDR;
    initialized = 1;
}

// 写入
int FlashCache_Write(const char *json, uint16_t len)
{
    if (!initialized) return -1;
    if (len > CACHE_ENTRY_SIZE - 12) return -2;

    CacheEntry_t entry;
		memset(&entry, 0xFF, sizeof(entry));		//擦除为1111
    entry.magic     = MAGIC_VALID;
    entry.timestamp = HAL_GetTick();
    entry.data_len  = len;
    memcpy(entry.data, json, len);

    // ─── 环形回绕处理 ───
    if (write_addr >= CACHE_END_ADDR) {   // 到达 8MB 边界
        write_addr = CACHE_START_ADDR;    // 回绕到 4MB
        SPI_FLASH_SectorErase(write_addr);// 擦除起始扇区
    }

    // ─── 扇区首地址擦除 ───
    if (write_addr % CACHE_SECTOR_SIZE == 0) {  // 写指针刚好在扇区边界
        SPI_FLASH_SectorErase(write_addr);      // 擦除整个 4KB 扇区
    }

    SPI_FLASH_BufferWrite((uint8_t *)&entry, write_addr, sizeof(CacheEntry_t));
    write_addr += sizeof(CacheEntry_t);	//固定128
    return 0;
}

// 读取一条，读完直接销毁（最安全，永不重复）
int FlashCache_ReadAndConsume(char *json, uint16_t *len)
{
    if (!initialized) return -1;

    CacheEntry_t entry;
    if (read_addr >= write_addr) {
        return -2; // 没有新数据
    }

    SPI_FLASH_BufferRead((uint8_t *)&entry, read_addr, sizeof(CacheEntry_t));
    if (entry.magic != MAGIC_VALID) {
        read_addr = write_addr;
        return -2;
    }

    // 读出数据
    memcpy(json, entry.data, entry.data_len);
    json[entry.data_len] = '\0';
    *len = entry.data_len;

    // 直接清空这条，防止重复读（最安全）
    entry.magic = MAGIC_EMPTY;
    SPI_FLASH_WriteEnable();
    SPI_FLASH_BufferWrite((uint8_t *)&entry, read_addr, sizeof(CacheEntry_t));

    read_addr += sizeof(CacheEntry_t);
    return 0;
}

// 是否有未读数据
// 【 fix 】判断是否有未读数据：只看 read < write，
// ======================================================================
int FlashCache_HasData(void)
{
    if (!initialized) return 0;

    // 最多只允许补传 5 条！！！
    // 强制限制，永远不会无限循环
    static int total = 0;
    if (total >= 5) {
        total = 0;
        return 0;
    }

    if (read_addr < write_addr) {		// 读指针落后写指针，说明有数据
        total++;
        return 1;
    }

    total = 0;
    return 0;
}

void my_clear()
{
	    write_addr = CACHE_START_ADDR;
    read_addr  = CACHE_START_ADDR;
}

// 清空
void FlashCache_Clear(void)
{
    for (uint32_t addr = CACHE_START_ADDR; addr < CACHE_END_ADDR; addr += CACHE_SECTOR_SIZE) {
        SPI_FLASH_SectorErase(addr);
    }
    write_addr = CACHE_START_ADDR;
    read_addr  = CACHE_START_ADDR;
    initialized = 1;
}

void MQTT_PublishJson(const char *json)
{
    uint8_t buf[256];
    int pos = 0;
    buf[pos++] = 0x32;                     /* PUBLISH, QoS 1 固定报头 报文类型 */

	uint16_t topic_len = 5;					//主题名长度
  uint16_t msg_len = strlen(json);
	uint16_t rem = 2 + topic_len + 2 + msg_len;				//2字节的主题长度+5字节的主题名+2字节的报文标识符+有效载荷长度

	/*变长剩余长度编码 低位在前 低7位表示数据 第八位(标志位)为1的话表示不止一个字节 小于128一个字节 最多4个字节*/
    do {
        uint8_t b = rem % 128;
        rem /= 128;
        if (rem > 0) b |= 0x80;
        buf[pos++] = b;
    } while (rem > 0);

		/*可变报头 两字节的主题长度 0x00 0x05  5字节的主题名 */
    buf[pos++] = 0;
    buf[pos++] = topic_len;
    memcpy(&buf[pos], "stm32", 5);
    pos += 5;

		/*两字节的报文标识符 0x00 0x01*/
    buf[pos++] = 0;
    buf[pos++] = 1;

		/*有效载荷*/
    memcpy(&buf[pos], json, msg_len);
    pos += msg_len;

    HAL_UART_Transmit(&huart1, buf, pos, 5000);
    vTaskDelay(500);
}

// 【 fix 】标记已发送：只改魔数，不擦除、不乱动指针
// ======================================================================
void FlashCache_MarkSent(void)
{
    if (!initialized) return;

    uint32_t cur = read_addr - sizeof(CacheEntry_t);	//找到前面补传数据的位置 
    if (cur < CACHE_START_ADDR) return;

    CacheEntry_t entry;
    SPI_FLASH_BufferRead((uint8_t *)&entry, cur, sizeof(CacheEntry_t));

    if (entry.magic == MAGIC_VALID) {
			entry.magic = MAGIC_EMPTY; // 只改魔数 不擦除扇区 标记已补传 下次不会再补传
        SPI_FLASH_WriteEnable();
        SPI_FLASH_BufferWrite((uint8_t *)&entry, cur, sizeof(CacheEntry_t));
    }
}

/*从读指针位置读一条缓存数据*/
int FlashCache_Read(char *json, uint16_t *len)
{
    if (!initialized) return -1;

    // 强制最多读 5 条，读完直接锁死
    static int read_cnt = 0;
    if (read_cnt >= 5) {
        read_cnt = 0;
        read_addr = write_addr;  // 👈 强制让读指针追上写指针 = 永远不再补传
        return -2;
    }

    if (read_addr >= write_addr) {
        read_cnt = 0;
        return -2;
    }

    CacheEntry_t entry;
    SPI_FLASH_BufferRead((uint8_t *)&entry, read_addr, sizeof(CacheEntry_t));

    if (entry.magic != MAGIC_VALID) {			/*只补传 这个值MAGIC_VALID*/
        read_addr = write_addr;
        read_cnt = 0;
        return -2;
    }

    memcpy(json, entry.data, entry.data_len);
    json[entry.data_len] = '\0';
    *len = entry.data_len;

    read_addr += sizeof(CacheEntry_t);//128
    read_cnt++;
    return 0;
}