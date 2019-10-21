#include "arg_pid.h"
#include "csp_adc.h"
#include "csp_timer.h"
#include "csp_pwm.h"
#include "csp_uart.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/*
 * 目标温度只可能在0-100.0度之间，不会出现零下情况，也不会出现100度
 * 以上的情况，所以当目标温度为0的时候默认不打开PID算法。
 * 所以关闭PID控制器的方法就是将目标温度设为0xffff度
 */
static bool pid_controller_sw[PID_CONTORLLER_NUM];
static int16_t pid_target_temp[PID_CONTORLLER_NUM];
static int16_t pid_now_temp[PID_CONTORLLER_NUM];
static int16_t pid_temp_error[PID_CONTORLLER_NUM];
static int16_t pid_temp_error_last[PID_CONTORLLER_NUM];

bool get_pid_con_sw(uint8_t id)
{
    return pid_controller_sw[id];
}

void set_pid_con_sw(uint8_t id, bool sw)
{
    pid_controller_sw[id] = sw;
}

void start_pid_controller_as_target_temp(uint8_t id , uint16_t target_temp){
    pid_target_temp[id % 10 ] = target_temp + 5;
    set_pid_con_sw(id,true);
}
void arg_pid_init(void)
{
    uint8_t i = 0;
    for (i = 0; i < PID_CONTORLLER_NUM; i++)
    {
        pid_target_temp[i] = 0xffff;
        pid_temp_error[i] = 0;
        pid_temp_error_last[i] = 0;
        pid_controller_sw[i] = false;
    }
}
//100ms进行一次pid运算
void arg_pid_handle(void)
{
    uint8_t i = 0;
    uint8_t pid_running_num = 0;

    uint8_t running_pid_id[2] = {0xff, 0xff};
    uint8_t running_pid_index = 0;
    uint16_t error_max = 0;
    uint8_t error_max_id = 0;

    float pwm_out1_f = 0;
    float pwm_out2_f = 0;

    float p_cal=0;
    float d_cal=0;
    uint16_t pwm_out1 = 0;
    uint16_t pwm_out2 = 0;

    #ifdef DEBUG_PID_SW
    uint8_t debug_sender_buf[200];    
    #endif 

    if (_PID_CONTROL_UP_FALG == false)
        return;

    _PID_CONTROL_UP_FALG = false;

    //更新一次所有温度的误差数据，如果出现降温情况那么关闭控制器

    for (i = 0; i < PID_CONTORLLER_NUM; i++)
    {
        pid_now_temp[i] = adc_temp_data[i];

        if (get_pid_con_sw(i) == true)
        {
            pid_temp_error[i] = pid_target_temp[i] - pid_now_temp[i];
        }
    }

    //同一时间只允许两个PID控制器起作用
    //竞争算法，首先判断哪些PID控制器起作用
    for (i = 0; i < PID_CONTORLLER_NUM; i++)
    {
        if (get_pid_con_sw(i) == true)
        {
            pid_running_num++;
            running_pid_id[running_pid_index % 2] = i;
            running_pid_index++;
        }
    }
    if (pid_running_num > 2)
    {
        //开始竞争算法,选出误差最大的两个左右优先项
        //对pid_temp_error进行分析，0xff的跳过，
        //从10个数据中挑出第一大和第二大的数据
        error_max = 0;
        for (i = 0; i < PID_CONTORLLER_NUM; i++)
        {
            if (error_max < pid_temp_error[i])
            {
                error_max = pid_temp_error[i];
                error_max_id = i;
            }
        }
        //获得了最大error的值

        running_pid_id[0] = error_max_id;

        //获得第二大的error id
        error_max = 0;
        for (i = 0; i < PID_CONTORLLER_NUM; i++)
        {
            if (i == running_pid_id[0])
                continue;

            if (error_max < pid_temp_error[i])
            {
                error_max = pid_temp_error[i];
                error_max_id = i;
            }
        }

        //获得了第二大的error id
        running_pid_id[1] = error_max_id;
    }

    //需要操控的id是running_pid_id[0] running_pid_id[1]两路，如果是0xff 0xff那么说明没有需要控制的PD控制器
    if(pid_running_num == 1){
        //关闭所有除了running_pid_id[0] running_pid_id[1]以外的所有设备的PWM
        for (i = 0; i < PID_CONTORLLER_NUM; i++)
        {
            set_software_pwm(i, 0);
        }        
        if (get_pid_con_sw(running_pid_id[0] % 10))
        {
            //开始计算PWM占空比，使用PD算法
            if (pid_temp_error[running_pid_id[0] % 10] > 0)
            {
                p_cal = (float)P * (float)pid_temp_error[running_pid_id[0] % 10];
                d_cal= (float)D * ((float)pid_temp_error_last[running_pid_id[0] % 10] - (float)pid_temp_error[running_pid_id[0]]);
                pwm_out1_f =  p_cal + d_cal;

                pid_temp_error_last[running_pid_id[0] % 10] = pid_temp_error[running_pid_id[0] % 10];

                pwm_out1 = (uint16_t)pwm_out1_f;

                if(pwm_out1 > 1000)
                    pwm_out1 = 1000;

                set_software_pwm(running_pid_id[0], pwm_out1);
            }else{
                pwm_out1 = 0;
                set_software_pwm(running_pid_id[0], pwm_out1);
            }
        #ifdef DEBUG_PID_SW
       // sprintf((char *)debug_sender_buf,"pid running controller num is %d\r\n,id is %d ,now temp = %d,target_temp = %d,control pwm = %d\r\n",pid_running_num,running_pid_id[0],pid_now_temp[running_pid_id[0]],pid_target_temp[running_pid_id[0]],pwm_out1);

       sprintf((char *)debug_sender_buf,"(%d,%d-------------%f,%f)\r\n",pid_now_temp[running_pid_id[0]],pwm_out1,p_cal,d_cal);
        debug_sender_str(debug_sender_buf);
        #endif             
        }
 
    }else{

        if (get_pid_con_sw(running_pid_id[0] % 10))
        {
            //关闭所有除了running_pid_id[0] running_pid_id[1]以外的所有设备的PWM
            for (i = 0; i < PID_CONTORLLER_NUM; i++)
            {
                set_software_pwm(i, 0);
            }            
            //开始计算PWM占空比，使用PD算法
            if (pid_temp_error[running_pid_id[0] % 10] > 0)
            {
                pwm_out1_f = (float)P * (float)pid_temp_error[running_pid_id[0] % 10] + (float)D * ((float)pid_temp_error_last[running_pid_id[0] % 10] - (float)pid_temp_error[running_pid_id[0]]);

                pid_temp_error_last[running_pid_id[0] % 10] = pid_temp_error[running_pid_id[0] % 10];

                pwm_out1 = (uint16_t)pwm_out1_f;

                if(pwm_out1 > 1000)
                    pwm_out1 = 1000;

                set_software_pwm(running_pid_id[0], pwm_out1);
            }
        }

        if (get_pid_con_sw(running_pid_id[1] % 10))
        {
            //开始计算PWM占空比，使用PD算法
            if (pid_temp_error[running_pid_id[1] % 10] > 0)
            {
                pwm_out2_f = (float)P * (float)pid_temp_error[running_pid_id[1] % 10] + (float)D * ((float)pid_temp_error_last[running_pid_id[1] % 10] - (float)pid_temp_error[running_pid_id[1]]);

                pid_temp_error_last[running_pid_id[1] % 10] = pid_temp_error[running_pid_id[1] % 10];

                pwm_out2 = (uint16_t)pwm_out2_f;

                if(pwm_out2 > 1000)
                    pwm_out2 = 1000;

                set_software_pwm(running_pid_id[1], pwm_out2);
            }
        }
        #ifdef DEBUG_PID_SW
        sprintf((char *)debug_sender_buf,\
					"pid running controller num is %d\r\n \
				,I : id is %d ,now temp = %d,target_temp = %d,control pwm = %d   \
				|||| II : id is %d ,now temp = %d,target_temp = %d,control pwm = %d\r\n" \
				,pid_running_num,\
				running_pid_id[0],pid_now_temp[running_pid_id[0]],pid_target_temp[running_pid_id[0]],pwm_out1,\
				running_pid_id[1],pid_now_temp[running_pid_id[1]],pid_target_temp[running_pid_id[1]],pwm_out2);
        debug_sender_str(debug_sender_buf);
        #endif                 
    }


  
}
