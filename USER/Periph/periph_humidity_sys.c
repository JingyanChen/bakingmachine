#include "periph_humidity_sys.h"
#include "csp_gpio.h"
#include "csp_timer.h"
#include <stdbool.h>
#include <stdint.h>
static injection_status_t injection_status; 
static bool needed_operate_box[5];
void periph_humidity_sys_init(void){
    uint8_t i=0;

    injection_status = no_injection_task;
    
    for(i=0;i<5;i++){
        needed_operate_box[i] = false;
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

void periph_humidity_sys_handle(void){
    uint8_t i=0;
    if(_HUMIDITY_CONTROL_FLAG == false)
        return ;

    _HUMIDITY_CONTROL_FLAG = false;

    if(injection_status == no_injection_task)
        return ;
    
    switch(injection_status){
        case injection_out_water: 
            //一直查询液位传感器响应
            for(i=0;i<5;i++){
                if(needed_operate_box[i] == true){
                    if(get_liquid_feedback_v(i) == LIQUID_FULL_V){
                        return ;
                    }
                }
            }

            //所有选中的路液位传感器 均不是FULL，说明已经倒空了

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
                    if(get_liquid_feedback_v(i) != LIQUID_FULL_V){
                        return ;
                    }
                }
            }

            //所有选中的路液位传感器 均是FULL，说明已经注满水了

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

        //执行此函数，说明至少有一路需要做换水操作

        if(b0 == false && b1 == false && b2 == false && b3 == false && b4 == false){
            return ;
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
