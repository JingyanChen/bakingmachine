#ifndef __HUMIDITY__
#define __HUMIDITY__


#include <stdbool.h>
#include <stdint.h>
/*
 * 本驱动旨在对湿度系统进行一个系统的封装
 * 湿度系统，旨在为第x路 x 属于 0-4 注满水
 * 确定需要哪路动作了之后，做如下操作
 * 
 * 1 打开出水阀
 * 2 打开第x路的蓄水槽的阀 ...
 * 3 打开出水泵
 * 4 抽水直到液位传感器认为没水为止
 * 5 关闭出水阀
 * 6 打开x路的蓄水槽阀 ...
 * 7 打开进水泵
 * 8 注水直到液位传感器认为水足够为止
 * 9 关闭进水阀
 * 
 * 
 * 外部文件的使用逻辑，使用periph_water_injection函数后
 * 一直等待get_water_injection_status()函数返回injection_done
 * 
 * 使用举例，第0路换水
 * 
 * periph_water_injection(true,false,false,false,false);
 * 
 * if(get_water_injection_status() == injection_done){
 *      //换水完毕
 *      periph_humidity_sys_init();//初始化一下，使得此if仅进入一次
 *      //if 进入一次的好处在于，条件成立的次数与调用periph_water_injection一致
 *      //可酌情考虑
 * }
 * 
 */


#define OUT_WATER_PUMP_ID 0
#define IN_WATER_PUMP_ID  1

#define IN_MASTER_VAVLE_ID 5
#define OUT_MASTER_VAVLE_ID 6

#define PUMP_ACTION_V  0
#define PUMP_NO_ACTION_V  1

#define VAVLE_ACTION_V 0
#define VAVLE_NO_ACTION_V 1
#define LIQUID_FULL_V 1


void periph_humidity_sys_init(void);
void periph_humidity_sys_handle(void);

typedef enum{
    no_injection_task=0,
    injection_out_water,
    injection_in_water,
    injection_done,
}injection_status_t;
/* 
 * 获得注水是否完成的状态
 */
injection_status_t get_water_injection_status(void);
/*
 * brief 注水代码
 * pra bool0 - bool4,决定五路盒子哪些需要做换水操作
 *      true  需要做
 *      false 不需要做
 */
void periph_water_injection(bool b0 ,bool b1 , bool b2 , bool b3 , bool b4); 


#endif
