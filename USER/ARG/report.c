#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "report.h"
#include <stdbool.h>
#include <stdint.h>
#include "csp_pwm.h"
#include "csp_timer.h"
#include "csp_gpio.h"
#include "csp_adc.h"
#include "csp_uart.h"
#include "delay.h"
#include "csp_wtd.h"

/*
 * 提供一个秒级别精度的计数器
 */
static uint32_t S_TICK=0;
static bool S_SW=false;
static void start_S_timer(void){
    S_TICK = 0;
    S_SW = true;
}

static void stop_S_timer(void){
    S_SW = false;
    S_TICK = 0;
}
static void S_timer_handle(void){
    if(S_SW){
       S_TICK ++; 
    }
}

static uint32_t get_S_timer(void){
    return S_TICK;
}

/*
 * 提供一个秒级别精度的计数器 end
 */






static bool is_all_ = false;
static uint8_t now_check_id = 0;
static bool HDR_START_LOGIC=false;
static uint16_t HDR_START_TEMP[2];
static check_status_t check_status;
static check_class_t check_class;

bool start_HRD_check(bool is_all){

    if(check_status != check_idle)
        return false;

    is_all_ = is_all;
    check_status = prepare_stage;
    check_class = HRD_CLASS;
    HDR_START_LOGIC = false;

    return true;
}

bool start_TCP_check(bool is_all){

    if(check_status != check_idle)
        return false;
    
    is_all_ = is_all;
    check_status = prepare_stage;
    check_class = TCP_CLASS;

    return true;
}

static void report_nop(void){
    csp_wtd_handle();
    delay_ms(10);
}

void arg_report_init(void){
    S_TICK=0;
    S_SW=false; 
    is_all_ = false;
    now_check_id = 0;
    check_status = check_idle;
    HDR_START_LOGIC = false;
}

void arg_report_handle(void){

    uint16_t road_temp_0 = 0;
    uint16_t road_temp_1 = 0;
    int16_t  road_temp_error=0;
    uint8_t  debug_sender_buf[200];
    float HDR_RATE=0.0;
    if(_ARG_REPORT_FLAG == false)
        return ;

    _ARG_REPORT_FLAG = false;

    S_timer_handle();

    switch(check_status){
        case check_idle: break;
        case prepare_stage:
            //两个测试的前提都是一致的，两边的温度必须保持低温且相近，如果不是
            //则需要打开水冷直到这个条件被达到
            water_cool_pump_control(now_check_id % 5, 0);

            road_temp_0 = adc_temp_data[(now_check_id * 2) % 10 ];
            road_temp_1 = adc_temp_data[(now_check_id * 2 + 1) % 10 ];

            //abs road_temp error
            if(road_temp_0 > road_temp_1){
                road_temp_error = road_temp_0 - road_temp_1;
            }else{
                road_temp_error = road_temp_1 - road_temp_0;
            }

            if( road_temp_0 < NORMAL_TEMP && 
                road_temp_1 < NORMAL_TEMP &&
                road_temp_error < TEMP_ERROR_MAX){
                    
                    //记录开始温度，为report做准备
                    HDR_START_TEMP[0] = road_temp_0;
                    HDR_START_TEMP[1] = road_temp_1;    

                    water_cool_pump_control(now_check_id % 5, 1);
                    check_status =  check_stage;

                    debug_sender_str("report : temp prepare success ...\r\n");report_nop();
                }
            break;
        case check_stage: 
            //检测阶段，先看是什么检测类型
            if(check_class == HRD_CLASS){
                if(HDR_START_LOGIC == false){
                //给出两路高温，并且开始计时
                    set_software_pwm((now_check_id * 2) % 10,1000);
                    set_software_pwm((now_check_id * 2 + 1) % 10,1000);

                    start_S_timer();
                    HDR_START_LOGIC = true;
                    debug_sender_str("report : temp up start success ...\r\n");report_nop();
                    //播种阶段
                }else{
                    if(get_S_timer() > HDR_CHECK_TIM){

                        set_software_pwm((now_check_id * 2) % 10,0);
                        set_software_pwm((now_check_id * 2 + 1) % 10,0);

                        //时间计数结束
                        stop_S_timer();//停止计时
                        
                        //上报HDR的检测结果
                        road_temp_0 = adc_temp_data[(now_check_id * 2) % 10 ];
                        road_temp_1 = adc_temp_data[(now_check_id * 2 + 1) % 10 ];
                        
                        road_temp_error = road_temp_0 - road_temp_1;
                        sprintf((char*)debug_sender_buf,"HDR REPORT ROAD %d\r\n",
                                now_check_id);
                        debug_sender_str(debug_sender_buf);report_nop();

                        sprintf((char*)debug_sender_buf,"using tim %d S full power operation temp from %d-%d to %d-%d\r\n",
                                HDR_CHECK_TIM,HDR_START_TEMP[0],HDR_START_TEMP[1],road_temp_0,road_temp_1);
                        debug_sender_str(debug_sender_buf);report_nop();

                        HDR_RATE = ((float)road_temp_0 - (float)road_temp_1) / (float)HDR_CHECK_TIM;
                        sprintf((char*)debug_sender_buf,"HDR = %d HDR_RATE = %.2f \r\n",
                                road_temp_error,HDR_RATE);
                        debug_sender_str(debug_sender_buf);report_nop();    

                        //报告完毕，回到初始状态机
                        check_status =  check_idle;

                        //判断是否是检测所有路的情况

                        if(is_all_ && now_check_id < 5){
                            now_check_id ++;
                           start_HRD_check(true); 
                        }else{
                            now_check_id = 0;
                            debug_sender_str("------HDR REPORT END-----\r\n");
                            HDR_START_LOGIC = false;
                        }
                    }
                    
                    //收获阶段
                }
            }else{

            }
            break;
    }
    
}
