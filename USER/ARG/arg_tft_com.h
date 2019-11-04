#ifndef __ARG_TFT_COM__
#define __ARG_TFT_COM__

#include <stdbool.h>
#include <stdint.h>
#include "arg_debug_pro.h"

void arg_tft_com_init(void);
void arg_tft_com_handle(void);

/*
 * decode原则
 *  
 * 所有来自上位机的指令码，遵循如下格式
 * 
 * 1 帧头 0xee
 * 2 数据长度 len
 * 3 功能码 cmd
 * 4 负载数据 load[]
 * 5 crc校验 crc
 * 6 帧尾 FF FD FE FF
 * 
 * 注意 其中 len的长度代表 功能码长度 + 负载数据长度
 *      其中 crc是校验 功能码 + 负载数据
 *      帧头帧尾是固定的
 * 
 * 单片机所有回复上位机的指令码，遵循如下格式
 * 
 * 1 帧头 0xee
 * 2 数据长度 len 
 * 3 功能码 cmd
 * 4 负载数据 load[]
 * 5 crc校验 crc
 * 6 帧尾 FF FD FE FF
 * 
 * 共同的数据包，但是负载数据根据功能码的不同意义不同
 * 详情见TFT_MCU通讯协议
 * 
 * 
 */


#define LOAD_MAX_LEN 20
typedef struct{
    uint8_t header;
    uint16_t len;
    uint16_t cmd;
    uint16_t load[LOAD_MAX_LEN];
    uint16_t crc;
    uint8_t tail[4];
}tft_mcu_pro_data_t;

// ee 00 01 00 01 00 11 crc crc ff fe fd ff
// ee 00 00 00 01 crc crc ff fe fd ff

typedef enum{
    open_temp_control=0,
    close_temp_control,
    write_fan,
    read_fan,
    write_box,
    read_box,
    read_now_temp,
    read_now_temp_target,
}cmd_func_list_t;

typedef void (*void_func_point_has_pra) (tft_mcu_pro_data_t * tft_mcu_pro_data);

typedef struct{
    void_func_point_has_pra func;
    uint16_t cmd;
}tft_com_func_list_t;


//open temp control error define

typedef enum{
    operate_succ=0,//操作成功
    box_not_closed,//盒子没有完全关闭
    pra_error,//参数错误
}open_temp_control_error_t;

#endif
