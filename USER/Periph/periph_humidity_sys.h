#ifndef __HUMIDITY__
#define __HUMIDITY__


#include <stdbool.h>
#include <stdint.h>

/*
 * 本驱动基于最新的注水/出水策略，对功能进行函数级的封装
 * 
 * 目前的注水/出水策略 -- 无液位传感器的策略
 * 
 * 开始实验之前，需要做一个换水操作，也就是抽完水之后，再重新注水
 * 
 * 做完实验之后，需要抽水
 * 
 * 所以本驱动提供两个上级函数
 * 
 * change_water(uint8_t liquid_road_list)
 * out_water(uint8_t liquid_road_list)
 * 
 * 执行上诉两个函数之后，会触发状态机，时刻提供此状态机的状态。
 * 
 * 提供 种状态
 * 
 * 1 no_injection_task
 * 2 change_water_out_status
 * 3 change_water_in_status
 * 4 change_water_done
 * 5 out_water_status
 * 6 out_water_done
 * 
 * 算法设计逻辑，应当是一次性做完所有的换水/出水操作，所以整个状态机仅做一个状态
 * 
 * 
 * 换水状态机 激活方法
 * change_water(0xff); //给所有路换水
 * out_water(0xff);//给所有路出水
 * 
 * if(change_water(0xff) == false){
 * 
 * }else{
 *      
 *      //打开轮询状态机查询状态机进展状态
 *      CHANGE_WATER_SUCCESS = TRUE;
 * }
 * 
 * if(CHANGE_WATER_SUCCESS){
 *      if(injection_status == change_water_done ){
 *              //换水结束
 *      }
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

#define WATER_ROAD_NUM 5

//算法重要参数，默认抽水 OUT_WATER_FIXED_TIM * 100 ms
#define OUT_WATER_FIXED_TIM 100

//算法重要参数，默认注水 IN_WATER_FIXED_TIM * 100 ms
#define IN_WATER_FIXED_TIM 30

void periph_humidity_sys_init(void);
void periph_humidity_sys_handle(void);

typedef enum{
    no_injection_task=0,
    change_water_out_status,
    change_water_in_status,
    change_water_done,
    out_water_status,
    out_water_done,
}injection_status_t;


injection_status_t get_water_injection_status(void);
void set_water_injection_status(injection_status_t injection_sta);


/*
 * brief : 换水代码，打开状态机，做换水操作
 * pra @ liquid_road_list : 水路的编号
 *      编号格式: BIT0 - BIT5 对应五路开关
 *      例如 0x01 则仅换0x01
 * 
 * 返回操作是否成功
 * 
 * 11.13 快速换水思考
 * 
 * 如果change_water被快速的重复调用
 * 那么状态机会置位，重新计数，不会对系统有任何影响
 * 对应的上位机操作是反复的访问换水状态
 * 
 * 换水逻辑和上位机的指令方式，需要整体联调
 * 因为存在总出水开关/总入水开关，所以一次性丢入所有需要控制
 * 的液路编号
 */
bool change_water(uint8_t liquid_road_list);

/*
 * brief : 清理水代码，做出水操作
 * pra @ liquid_road : 水路编号
 * return 出水操作是否成功
 */
bool out_water(uint8_t liquid_road_list);

//获得液路开启情况
bool is_water_road_open(uint8_t water_road_id);

#endif
