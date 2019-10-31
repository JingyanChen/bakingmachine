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


typedef void (*void_func_point) (void);

#define MAX_DEBUG_FUNCTION_LEN 100
typedef struct{
    void_func_point func;
    char cmd_str[MAX_DEBUG_FUNCTION_LEN];
}debug_func_list_t;

/*
 * brief : 获得用户设置的TFT屏DEBUG开关信号
 * 如果用户打开DEBUG功能，则可以观察
 * TFT-MCU之间的UART通讯包
 * 不支持DEBUG功能干预通讯，有任何实际写操作
 * 只允许DEBUG口观察，监控只读
 */
bool get_tft_com_transmit_sw(void);
#endif
