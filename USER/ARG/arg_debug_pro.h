#ifndef __ARG_DEBUG__
#define __ARG_DEBUG__

#include <stdint.h>
#include <stdbool.h>

/*
 * 本驱动旨在通过DEBUG_UART 实现一个指令列表，来测试系统各方面的性能
 * 抽象的层级
 * 
 * 1 CSP层级抽象
 * 
 *   1 获得十路电压值，以判断ADC设备是否工作正常 
 *     get_csp_adc
 *   2 设置温控PWM占空比
 *     set_warm_pwm id percent
 *   3 设置电机占空比
 *     set_motor_pwm id percent
 *   4 
 * 2 periph层级抽象
 *     //电机运动抽象
 *     //..
 * 
 */

void arg_debug_pro_init(void);
void arg_debug_pro_handle(void);

#define MAX_STRING_DECODE_PRA_NUM 20
typedef struct{
    bool is_vaild_string;
    uint32_t pra_num;
    uint32_t pra_list[MAX_STRING_DECODE_PRA_NUM];
}string_decode_t;

typedef void (*void_func_point) (string_decode_t * string_decode_result);

#define MAX_DEBUG_FUNCTION_LEN 100
typedef struct{
    void_func_point func;
    char cmd_str[MAX_DEBUG_FUNCTION_LEN];
    uint8_t pra_num;
}debug_func_list_t;

/*
 * brief : 获得用户设置的TFT屏DEBUG开关信号
 * 如果用户打开DEBUG功能，则可以观察
 * TFT-MCU之间的UART通讯包
 * 不支持DEBUG功能干预通讯，有任何实际写操作
 * 只允许DEBUG口观察，监控只读
 */
bool get_tft_com_transmit_sw(void);

/*
 * 获得box运动时的变速信息，方便调试使用,目前仅支持查看第一路的信息
 */
bool get_box_running_debug_sw(void);

/*
 * 获得当前的PID控制状态
 */
bool get_pid_debug_sw(void);

/*
 * 20.1.9 加入功能函数，对于如下格式字符串进行解析
 * function_name pra1 pra2 ... \r\n
 * 解析返回字符串格式是否正确，并且解析合法的变量个数，返回参数表
 * 解析的最大值为uint32_t的数据类型
 * 最多支持 MAX_STRING_DECODE_PRA_NUM 个参数表的解析
 */
string_decode_t decode_string(uint8_t * str , uint16_t str_len);

#endif
