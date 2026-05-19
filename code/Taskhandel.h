#ifndef TASKHANDEL_H
#define TASKHANDEL_H
#include "stdint.h"




void MQTT();
void AT();
void UI();
void Modbus();


// 温湿度数据结构体

typedef struct {
    float temp;   // 温度
    float shi;    // 湿度
} Data_t;



#endif
