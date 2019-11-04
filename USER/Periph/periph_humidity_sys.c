#include "periph_humidity_sys.h"
#include "csp_gpio.h"
#include "csp_timer.h"
#include <stdbool.h>
#include <stdint.h>
#include "csp_uart.h"

static injection_status_t injection_status;
static bool water_op_sw[WATER_ROAD_NUM];
static uint16_t out_water_tim_tick;
static uint16_t in_water_tim_tick;

void periph_humidity_sys_init(void)
{
    uint8_t i = 0;
    for (i = 0; i < WATER_ROAD_NUM; i++)
    {
        water_op_sw[i] = false;
    }
    out_water_tim_tick = 0;
    in_water_tim_tick = 0;
    injection_status = no_injection_task;

    humidity_control(OUT_WATER_PUMP_ID,VAVLE_NO_ACTION_V);
    humidity_control(IN_WATER_PUMP_ID,VAVLE_NO_ACTION_V);

}

void periph_humidity_sys_handle(void)
{
    uint8_t i = 0;

    if (_HUMIDITY_CONTROL_FLAG == false)
        return;

    _HUMIDITY_CONTROL_FLAG = false;

    if (injection_status == no_injection_task ||
        injection_status == change_water_done ||
        injection_status == out_water_done)
    {
        return;
    }

    //有需要计时的事件
    switch (injection_status)
    {
    case change_water_out_status:
        out_water_tim_tick++;
        if (out_water_tim_tick > OUT_WATER_FIXED_TIM)
        {

            //抽水时间到，关闭抽水阀
            water_cool_vavle_control(OUT_MASTER_VAVLE_ID, VAVLE_NO_ACTION_V);
            humidity_control(OUT_WATER_PUMP_ID, PUMP_NO_ACTION_V);

            //配置注水阀/泵操作
            water_cool_vavle_control(IN_MASTER_VAVLE_ID, VAVLE_ACTION_V);
            humidity_control(IN_WATER_PUMP_ID, PUMP_ACTION_V);

            injection_status = change_water_in_status;
        }
        break;
    case change_water_in_status:
        in_water_tim_tick++;
        if (in_water_tim_tick > IN_WATER_FIXED_TIM)
        {

            //注水时间到，关闭注水阀
            water_cool_vavle_control(IN_MASTER_VAVLE_ID, VAVLE_NO_ACTION_V);
            humidity_control(IN_WATER_PUMP_ID, PUMP_NO_ACTION_V);
            
            //关闭所有阀，换水操作结束

            for (i = 0; i < WATER_ROAD_NUM; i++)
            {
                out_water_tim_tick = 0;
                in_water_tim_tick = 0;
                water_cool_vavle_control(i, VAVLE_NO_ACTION_V);
            }

            injection_status = change_water_done;
        }
        break;
    case change_water_done:

        //中间状态，复位状态机
        for (i = 0; i < WATER_ROAD_NUM; i++)
        {
            water_op_sw[i] = false;
        }

        out_water_tim_tick = 0;
        in_water_tim_tick = 0;      

        break;
    case out_water_status:
        out_water_tim_tick++;
        if (out_water_tim_tick > OUT_WATER_FIXED_TIM)
        {

            //抽水时间到，关闭抽水阀
            water_cool_vavle_control(OUT_MASTER_VAVLE_ID, VAVLE_NO_ACTION_V);
            humidity_control(OUT_WATER_PUMP_ID, PUMP_NO_ACTION_V);

            //关闭所有阀，换水操作结束

            for (i = 0; i < WATER_ROAD_NUM; i++)
            {
                out_water_tim_tick = 0;
                in_water_tim_tick = 0;
                water_cool_vavle_control(i, VAVLE_NO_ACTION_V);
            }

            injection_status = out_water_done;
        }
        break;
    case out_water_done:
        //中间状态，复位状态机
        for (i = 0; i < WATER_ROAD_NUM; i++)
        {
            water_op_sw[i] = false;
        }

        out_water_tim_tick = 0;
        in_water_tim_tick = 0;
        break;
    
    default : 
    
        break;
    }
}

injection_status_t get_water_injection_status(void)
{
    return injection_status;
}

void set_water_injection_status(injection_status_t injection_sta){
    injection_status = injection_sta;
}
bool change_water(uint8_t liquid_road_list)
{
    uint8_t i = 0;

    if (liquid_road_list == 0x00)
    {
        return false; //没有有效的数据
    }

    if (injection_status != no_injection_task && injection_status != change_water_done && injection_status != out_water_done)
    {
        //出现了如下情况，上一次换水并未结束，又开始新的一次换水
        //初始化状态机，重置状态机。
        //表现是 如果快速的换水，不会对系统有任何影响
        periph_humidity_sys_init();
    }


    for (i = 0; i < WATER_ROAD_NUM; i++)
    {
        if (liquid_road_list & (0x01 << i))
        {
            //配置操作做换水操作，状态机
            water_op_sw[i] = true;

            out_water_tim_tick = 0;
            in_water_tim_tick = 0;

            //打开对应的槽阀
            water_cool_vavle_control(i, VAVLE_ACTION_V);
        }
    }

    //开总阀

    water_cool_vavle_control(OUT_MASTER_VAVLE_ID, VAVLE_ACTION_V);

    //打开出水泵
    humidity_control(OUT_WATER_PUMP_ID, PUMP_ACTION_V);

    //配置完所有需要操作的水路之后，做统一的状态机操作，注册到下一个状态机

    injection_status = change_water_out_status;
		
	return true;
}

bool out_water(uint8_t liquid_road_list)
{
        uint8_t i = 0;

    if (liquid_road_list == 0x00)
    {
        return false; //没有有效的数据
    }

    if (injection_status != no_injection_task && injection_status != change_water_done && injection_status != out_water_done)
    {
        //出现了如下情况，上一次换水并未结束，又开始新的一次换水
        //初始化状态机，重置状态机。
        //表现是 如果快速的退水，不会对系统有任何影响
        periph_humidity_sys_init();
    }

    for (i = 0; i < WATER_ROAD_NUM; i++)
    {
        if (liquid_road_list & (0x01 << i))
        {
            //配置操作做抽水操作，状态机
            water_op_sw[i] = true;

            out_water_tim_tick = 0;
            in_water_tim_tick = 0;

            //打开对应的槽阀
            water_cool_vavle_control(i, VAVLE_ACTION_V);
        }
    }

    //开总阀

    water_cool_vavle_control(OUT_MASTER_VAVLE_ID, VAVLE_ACTION_V);

    //打开出水泵
    humidity_control(OUT_WATER_PUMP_ID, PUMP_ACTION_V);

    //配置完所有需要操作的水路之后，做统一的状态机操作，注册到下一个状态机

    injection_status = out_water_status;
		
	return true;
}


bool is_water_road_open(uint8_t water_road_id){
    return water_op_sw[water_road_id % WATER_ROAD_NUM];
}
