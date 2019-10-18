#include "arg_pid.h"
#include "csp_adc.h"
#include "csp_timer.h"
#include "csp_pwm.h"
/*
 * 目标温度只可能在0-100.0度之间，不会出现零下情况，也不会出现100度
 * 以上的情况，所以当目标温度为0的时候默认不打开PID算法。
 * 所以关闭PID控制器的方法就是将目标温度设为0xffff度
 */
uint16_t pid_target_temp[PID_CONTORLLER_NUM];
uint16_t pid_now_temp[PID_CONTORLLER_NUM];
uint16_t pid_temp_error[PID_CONTORLLER_NUM];
uint16_t pid_temp_error_last[PID_CONTORLLER_NUM];

void arg_pid_init(void)
{
    uint8_t i = 0;
    for (i = 0; i < PID_CONTORLLER_NUM; i++)
    {
        pid_target_temp[i] = 0xffff;
        pid_temp_error[i] = 0;
        pid_temp_error_last[i] = 0;
    }
}
//10ms进行一次pid运算
void arg_pid_handle(void)
{
    uint8_t i = 0;
    uint8_t pid_running_num = 0;

    uint8_t running_pid_id[2]={0xff,0xff};
    uint8_t running_pid_index = 0;
    uint16_t error_max = 0;
    uint8_t error_max_id=0;

    float pwm_out1_f=0;
    float pwm_out2_f=0;

    uint16_t pwm_out1=0;
    uint16_t pwm_out2=0;

    if (_PID_CONTROL_UP_FALG == false)
        return;
    _PID_CONTROL_UP_FALG = false;

    //更新一次所有温度的误差数据，如果出现降温情况那么关闭控制器

    for (i = 0; i < PID_CONTORLLER_NUM; i++)
    {
        pid_now_temp[i] = adc_temp_data[i];

        if (pid_target_temp[i] != 0xff)
        {

            if (pid_target_temp[i] < pid_now_temp[i])
            {
                pid_target_temp[i] = 0xff; //开始降温了关闭PD控制器
                pid_temp_error[i] = 0;
            }
            else
            {
                pid_temp_error[i] = pid_target_temp[i] - pid_now_temp[i];
            }
        }
    }

    //同一时间只允许两个PID控制器起作用
    //竞争算法，首先判断哪些PID控制器起作用
    for (i = 0; i < PID_CONTORLLER_NUM; i++)
    {
        if (pid_target_temp[i] != 0xff)
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
            if(error_max < pid_temp_error[i]){
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
            if(i == running_pid_id[0])
                continue;

            if(error_max < pid_temp_error[i]){
                error_max = pid_temp_error[i];
                error_max_id = i;
            }
        }

        //获得了第二大的error id
        running_pid_id[1] = error_max_id;       
    }

    //关闭所有除了running_pid_id[0] running_pid_id[1]以外的所有设备的PWM
        for (i = 0; i < PID_CONTORLLER_NUM; i++)
        {
            set_software_pwm(i,0);
        }


    //需要操控的id是running_pid_id[0] running_pid_id[1]两路，如果是0xff 0xff那么说明没有需要控制的PD控制器

    if(running_pid_id[0] != 0xff){
        //开始计算PWM占空比，使用PD算法
        
       pwm_out1_f = (float)P * (float)pid_temp_error[running_pid_id[0] % 10] + (float)D * ((float)pid_temp_error_last[running_pid_id[0] % 10] - (float)pid_temp_error[running_pid_id[0]]);

       pid_temp_error_last[running_pid_id[0] % 10] = pid_temp_error[running_pid_id[0] % 10] ;

        pwm_out1 = (uint16_t)pwm_out1_f;

       set_software_pwm(running_pid_id[0],pwm_out1 % 1000);
    }

    if(running_pid_id[1] != 0xff){
        //开始计算PWM占空比，使用PD算法

       pwm_out2_f = (float)P * (float)pid_temp_error[running_pid_id[1] % 10] + (float)D * ((float)pid_temp_error_last[running_pid_id[1] % 10] - (float)pid_temp_error[running_pid_id[1]]);

       pid_temp_error_last[running_pid_id[1] % 10] = pid_temp_error[running_pid_id[1] % 10] ;

        pwm_out2 = (uint16_t)pwm_out2_f;

       set_software_pwm(running_pid_id[1],pwm_out2 % 1000);
    }
}
