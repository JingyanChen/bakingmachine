#include "periph_humidity_sys.h"
#include "csp_gpio.h"
#include "csp_timer.h"
#include <stdbool.h>
#include <stdint.h>
#include "csp_uart.h"

static injection_status_t injection_status; 
static bool needed_operate_box[5];
static bool out_water_done[5];
static uint16_t out_water_tube_has_no_water_tick[5];

static bool in_water_done[5];
static uint16_t in_water_tube_has_water_tick[5];
static uint16_t in_water_tube_has_no_water_tick[5];

void periph_humidity_sys_init(void){
    uint8_t i=0;

    injection_status = no_injection_task;
    
    for(i=0;i<5;i++){
        needed_operate_box[i] = false;
        out_water_tube_has_no_water_tick[i] = 0;
        out_water_done[i] = 0;
        in_water_done[i] = 0;
        in_water_tube_has_water_tick[i] = 0;
        in_water_tube_has_no_water_tick[i] = 0;
    }

    //关闭出水泵
    humidity_control(OUT_WATER_PUMP_ID,PUMP_NO_ACTION_V);
            
    //关闭出水总阀
    water_cool_vavle_control(OUT_MASTER_VAVLE_ID,VAVLE_NO_ACTION_V);

    //关入水阀
    water_cool_vavle_control(IN_MASTER_VAVLE_ID,VAVLE_NO_ACTION_V);

    //关入水泵
    humidity_control(IN_WATER_PUMP_ID,PUMP_NO_ACTION_V);            
            
    //关闭所有阀
    for(i=0;i<5;i++){
        water_cool_vavle_control(i,VAVLE_NO_ACTION_V);
    }    
}

static void error_indicator_default_func(void){
    debug_sender_str("out water error \r\n");
}

error_indicator error_indicator_func = error_indicator_default_func;
void register_liquid_error_indicator(error_indicator func){
    error_indicator_func = func;
}

void periph_humidity_sys_handle(void){
    uint8_t i=0,j=0;
    if(_HUMIDITY_CONTROL_FLAG == false)
        return ;

    _HUMIDITY_CONTROL_FLAG = false;

    if(injection_status == no_injection_task)
        return ;
    
    switch(injection_status){
        case injection_out_water: 
            //查询管道里是否有水
            //算法简单来说就是“抽水一直抽到管道无水超过5S”
            for(i=0;i<5;i++){
                if(needed_operate_box[i] == true){
                    //
                    if(get_liquid_feedback_v(i) == LIQUID_HAS_WATER_V){
                        out_water_tube_has_no_water_tick[i] = 0;
                    }else{
                        out_water_tube_has_no_water_tick[i]++;
                        if(out_water_tube_has_no_water_tick[i] > LIQUID_NO_WATER_OUT_DONE_OVERTIME){
                            out_water_done[i] = true;
                        }

                    }
                }
            }
            //打开了抽水任务，并且所有路皆抽水完成，则进入下一个环节
            for(i=0;i<5;i++){
                if(needed_operate_box[i] == true && out_water_done[i] == false){
                    //出现了需要抽水但没有抽完的情况，返回
                    return ;
                }
            }

            //所有抽水环节全部OK，第一步还原状态机
            for(i=0;i<5;i++){
                out_water_tube_has_no_water_tick[i] = 0;
                out_water_done[i] = false;
            }


            //关闭出水泵
            humidity_control(OUT_WATER_PUMP_ID,PUMP_NO_ACTION_V);
            
            //关闭出水总阀
            water_cool_vavle_control(OUT_MASTER_VAVLE_ID,VAVLE_NO_ACTION_V);

            //开入水阀
            water_cool_vavle_control(IN_MASTER_VAVLE_ID,VAVLE_ACTION_V);

            //开入水泵
            humidity_control(IN_WATER_PUMP_ID,PUMP_ACTION_V);

            //激活状态机，下一个等待传感器响应注满水
            injection_status = injection_in_water;
            break;

        case injection_in_water: 

            //一直查询液位传感器响应
            for(i=0;i<5;i++){
                if(needed_operate_box[i] == true){
                    if(get_liquid_feedback_v(i) != LIQUID_HAS_WATER_V){
                        in_water_tube_has_no_water_tick[i] = 0;
                        in_water_tube_has_water_tick[i]++;
                        if(in_water_tube_has_water_tick[i] > LIQUID_IN_WATER_ALL_TIM_MS){
                            //注水3S时间到
                            in_water_done[i] = true;
                        }
                    }else{
                        in_water_tube_has_no_water_tick[i]++;
                        if(in_water_tube_has_no_water_tick[i] > LIQUID_IN_WATER_ERROR_TIM){
                            //出现错误情况，在抽水时间里超过1S没抽到水
                            //指示错误，复位状态机
                            injection_status = no_injection_task;
                            
                            for(j=0;j<5;j++){
                                needed_operate_box[j] = false;
                                out_water_tube_has_no_water_tick[j] = 0;
                                out_water_done[j] = 0;
                                in_water_done[j] = 0;
                                in_water_tube_has_water_tick[j] = 0;
                                in_water_tube_has_no_water_tick[j] = 0;
                            }

                            error_indicator_func();
                            return ;
                        }
                    }
                }
            }

            //判断是否所有都注水完毕
            for(i=0;i<5;i++){
                if(needed_operate_box[i] == true && in_water_done[i] == false){
                    //出现了需要注水但没有注完的情况，返回
                    return ;
                }
            }            



            //关入水阀
            water_cool_vavle_control(IN_MASTER_VAVLE_ID,VAVLE_NO_ACTION_V);

            //关入水泵
            humidity_control(IN_WATER_PUMP_ID,PUMP_NO_ACTION_V);            
            
            //关闭所有阀
            for(i=0;i<5;i++){
                water_cool_vavle_control(i,VAVLE_NO_ACTION_V);
            }

            //完成操作
            injection_status = injection_done;
            break;
        case injection_done: break;
        default : break;
    }
}

injection_status_t get_water_injection_status(void){
    return injection_status;
}

void periph_water_injection(bool b0 , bool b1 , bool b2 , bool b3 , bool b4){

        uint8_t i=0;

        //执行此函数，说明至少有一路需要做换水操作

        if(b0 == false && b1 == false && b2 == false && b3 == false && b4 == false){
            return ;
        }

        injection_status = no_injection_task;
        
        for(i=0;i<5;i++){
            needed_operate_box[i] = false;
            out_water_tube_has_no_water_tick[i] = 0;
            out_water_done[i] = 0;
            in_water_done[i] = 0;
            in_water_tube_has_water_tick[i] = 0;
            in_water_tube_has_no_water_tick[i] = 0;
        }

        //按照需要打开b1-b5的阀

        if(b0)  {water_cool_vavle_control(0,VAVLE_ACTION_V);needed_operate_box[0]=true;}
        if(b1)  {water_cool_vavle_control(1,VAVLE_ACTION_V);needed_operate_box[1]=true;}
        if(b2)  {water_cool_vavle_control(2,VAVLE_ACTION_V);needed_operate_box[2]=true;}
        if(b3)  {water_cool_vavle_control(3,VAVLE_ACTION_V);needed_operate_box[3]=true;}
        if(b4)  {water_cool_vavle_control(4,VAVLE_ACTION_V);needed_operate_box[4]=true;}
        
        //打开出水总阀

        water_cool_vavle_control(OUT_MASTER_VAVLE_ID,VAVLE_ACTION_V);

        //打开出水泵

        humidity_control(OUT_WATER_PUMP_ID,PUMP_ACTION_V);

        //等待液位传感器回归数值,注册下一个状态
        injection_status = injection_out_water;

}
