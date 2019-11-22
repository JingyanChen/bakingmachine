#include "arg_pid.h"
#include "csp_adc.h"
#include "csp_timer.h"
#include "csp_pwm.h"
#include "csp_uart.h"
#include "csp_gpio.h"
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

//温控模式相关处理代码

temp_control_mode_t temp_control_mode;

static void set_temp_control_mode(temp_control_mode_t mode)
{
    temp_control_mode = mode;
}

static temp_control_mode_t get_temp_control_mode(void)
{
    return temp_control_mode;
}

//end温控模式相关处理代码

bool get_pid_con_sw(uint8_t id)
{
    return pid_controller_sw[id];
}

void set_pid_con_sw(uint8_t id, bool sw)
{
    pid_controller_sw[id] = sw;
}

void set_pid_controller_mode_as_decentralize(uint8_t id, uint16_t target_temp)
{

    pid_target_temp[(id) % 10] = target_temp + 5;
    pid_target_temp[(id * 2 + 1) % 10] = target_temp + 5;

    set_pid_con_sw(id % 10, true);
    set_pid_con_sw((id * 2 + 1) % 10, true);

    set_temp_control_mode(DECENTRALIZED_CONTROL_MODE);
}

static uint16_t concentrate_control_mode_road_id[2];
static bool concentrate_condition_done = false;
static bool decentralize_busy_flag = false;

bool get_concentrate_status(void){
    return concentrate_condition_done;
}
bool get_decentralize_busy_flag(void){
    return decentralize_busy_flag;
}

bool set_pid_controller_mode_as_concentrate(uint8_t road_id, uint16_t target_temp)
{
    if(decentralize_busy_flag == true){
        return false;
    }

    set_pid_controller_mode_as_decentralize(road_id, target_temp);
    set_temp_control_mode(CONCENTRATE_CONTROL_MODE);

    concentrate_control_mode_road_id[0] = road_id;
    concentrate_control_mode_road_id[1] = road_id * 2 + 1;

    concentrate_condition_done  = false;

    return true;
}



/*
 * 分散升温模式，仅用在升温状态下
 * 此handle被定时执行，那么所有sw被打开的路都会被分配温度资源
 * 且分配的原则是，谁与目标差别最大，谁获得资源
 */
void decentralized_control_mode_handle(void)
{
    uint8_t i = 0;
    uint8_t pid_running_num = 0;

    uint8_t running_pid_id[2] = {0xff, 0xff};
    uint8_t running_pid_index = 0;
    uint16_t error_max = 0;
    uint8_t error_max_id = 0;

    float pwm_out1_f = 0;
    float pwm_out2_f = 0;

    uint16_t pwm_out1 = 0;
    uint16_t pwm_out2 = 0;

    #ifdef DEBUG_PID_SW
    uint8_t debug_sender_buf[200];
    #endif

    //更新一次所有温度的误差数据

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

    if (get_pid_con_sw(running_pid_id[0] % 10))
    {
        if (pid_temp_error[running_pid_id[0] % 10] > 0)
        {
            //开始计算PWM占空比，使用PD算法
            pwm_out1_f = (float)P * (float)pid_temp_error[running_pid_id[0] % 10] + (float)D * ((float)pid_temp_error_last[running_pid_id[0] % 10] - (float)pid_temp_error[running_pid_id[0]]);

            pid_temp_error_last[running_pid_id[0] % 10] = pid_temp_error[running_pid_id[0] % 10];

            pwm_out1 = (uint16_t)pwm_out1_f;

            if (pwm_out1 > 1000)
                pwm_out1 = 1000;

            set_software_pwm(running_pid_id[0], pwm_out1);
        }
        else
        {
            //出现降温的情况，直接给到0
            pwm_out1 = 0;
            set_software_pwm(running_pid_id[0], pwm_out1);
        }
    }

    if (get_pid_con_sw(running_pid_id[1] % 10))
    {
        if (pid_temp_error[running_pid_id[1] % 10] > 0)
        {
            //开始计算PWM占空比，使用PD算法
            pwm_out2_f = (float)P * (float)pid_temp_error[running_pid_id[1] % 10] + (float)D * ((float)pid_temp_error_last[running_pid_id[1] % 10] - (float)pid_temp_error[running_pid_id[1]]);

            pid_temp_error_last[running_pid_id[1] % 10] = pid_temp_error[running_pid_id[1] % 10];

            pwm_out2 = (uint16_t)pwm_out2_f;

            if (pwm_out2 > 1000)
                pwm_out2 = 1000;

            set_software_pwm(running_pid_id[1], pwm_out2);
        }
        else
        {
            pwm_out2 = 0;
            set_software_pwm(running_pid_id[1], pwm_out2);
        }
    }

    #ifdef DEBUG_PID_SW
    sprintf((char *)debug_sender_buf,
            "pid running controller num is %d\r\n \
				,I : id is %d ,now temp = %d,target_temp = %d,control pwm = %d   \
				|||| II : id is %d ,now temp = %d,target_temp = %d,control pwm = %d\r\n",
            pid_running_num,
            running_pid_id[0], pid_now_temp[running_pid_id[0]], pid_target_temp[running_pid_id[0]], pwm_out1,
            running_pid_id[1], pid_now_temp[running_pid_id[1]], pid_target_temp[running_pid_id[1]], pwm_out2);
    debug_sender_str(debug_sender_buf);
    #endif

    
    //判断所有打开的温度控制路，是否都处于控温状态了，如果是的，把BUSY位置0，告诉上级，处于NO BUSY状态
    //可以响应新的升温任务
    //对控温状态的定义：目标温度 - 当前温度   < 1.0摄氏度
    //举例  目标60.0°  59.0摄氏度一上就认为处于控温区间了，予以放行

    for (i = 0; i < PID_CONTORLLER_NUM; i++)
    {
        if (get_pid_con_sw(i) == true)
        {
            if(pid_temp_error[i] < 10)
            {

            }else{
                decentralize_busy_flag = true;
                return ;
            } 
        }
    }

    decentralize_busy_flag = false;

}

/*
 * 集中升温模式
 * 触发此模式，必须实现配置好需要集中控制的road id号
 * 此升温模式是暂时的短暂的升温模式，只要一次到达目标温度，那么升温模式就结束了
 */
void concentrate_control_mode_handle(void)
{
    uint8_t i = 0;

    float pwm_out1_f = 0;
    float pwm_out2_f = 0;

    uint16_t pwm_out1 = 0;
    uint16_t pwm_out2 = 0;

    #ifdef DEBUG_PID_SW
    uint8_t debug_sender_buf[200];
    #endif

    //更新一次所有温度的误差数据

    for (i = 0; i < PID_CONTORLLER_NUM; i++)
    {
        pid_now_temp[i] = adc_temp_data[i];

        //仅更新集中体现的error
        if (i == concentrate_control_mode_road_id[0] ||
            i == concentrate_control_mode_road_id[1])
        {
            pid_temp_error[i] = pid_target_temp[i] - pid_now_temp[i];
        }
    }

    //开始PID控温
    if (pid_temp_error[concentrate_control_mode_road_id[0] % 10] > 0)
    {

        pwm_out1_f = (float)P * (float)pid_temp_error[concentrate_control_mode_road_id[0] % 10] + (float)D * ((float)pid_temp_error_last[concentrate_control_mode_road_id[0] % 10] - (float)pid_temp_error[concentrate_control_mode_road_id[0]]);

        pid_temp_error_last[concentrate_control_mode_road_id[0] % 10] = pid_temp_error[concentrate_control_mode_road_id[0] % 10];

        pwm_out1 = (uint16_t)pwm_out1_f;

        if (pwm_out1 > 1000)
            pwm_out1 = 1000;

        set_software_pwm(concentrate_control_mode_road_id[0], pwm_out1);
    }else
    {
        pwm_out1 = 0;

        set_software_pwm(concentrate_control_mode_road_id[0], pwm_out1);        
    }

    if (pid_temp_error[concentrate_control_mode_road_id[1] % 10] > 0)
    {
        pwm_out2_f = (float)P * (float)pid_temp_error[concentrate_control_mode_road_id[1] % 10] + (float)D * ((float)pid_temp_error_last[concentrate_control_mode_road_id[1] % 10] - (float)pid_temp_error[concentrate_control_mode_road_id[1]]);

        pid_temp_error_last[concentrate_control_mode_road_id[1] % 10] = pid_temp_error[concentrate_control_mode_road_id[1] % 10];

        pwm_out2 = (uint16_t)pwm_out2_f;

        if (pwm_out2 > 1000)
            pwm_out2 = 1000;

        set_software_pwm(concentrate_control_mode_road_id[1], pwm_out2);
    }else{

        pwm_out2 = 0;

        set_software_pwm(concentrate_control_mode_road_id[0], pwm_out1);            
    }

    //TODO 目前集中升温模式的结束标志仅仅是瞬间到达目标温度
    //后期可以再考虑结束的标志，结束标志决定是否进入控温态
    if(pid_temp_error[concentrate_control_mode_road_id[0] % 10] < 0 &&
       pid_temp_error[concentrate_control_mode_road_id[1] % 10] < 0 ){

        //两个都到达温度了，集中温度模式结束
        concentrate_condition_done = true;

        //自动切换进入分散温度控制模式
        set_temp_control_mode(DECENTRALIZED_CONTROL_MODE);
        
        concentrate_condition_done = true;
       }
}

static bool have_temp_control_task(void)
{

    uint8_t i = 0;

    for (i = 0; i < PID_CONTORLLER_NUM; i++)
    {
        if (pid_controller_sw[i] == true)
        {
            return true;
        }
    }

    return false;
}

static uint16_t water_cool_road_id =0;
static uint16_t water_cool_target_temp=0;
static bool water_cool_check_start=false;
void start_water_cool(uint8_t road_id ,uint16_t target_temp){

    water_cool_pump_control(road_id % 5, 0 );//打开水冷降温
    water_cool_road_id = road_id;
    water_cool_check_start = true;

}
void water_cool_init(void){
    water_cool_check_start = false;
    water_cool_road_id = 0;
    water_cool_target_temp = 0;
}
void water_cool_handle(void){
    uint16_t water_cool_road_temp = 0;

    if(water_cool_check_start == false)
        return ;

    water_cool_road_temp += adc_temp_data[water_cool_road_id % 10];
    water_cool_road_temp += adc_temp_data[(water_cool_road_id * 2 + 1) % 10];

    water_cool_road_temp /= 2;


    if(water_cool_road_temp < water_cool_target_temp){
        water_cool_pump_control(water_cool_road_id % 5, 1 );//关闭水冷降温
        water_cool_init();
    }
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

    set_temp_control_mode(CONCENTRATE_CONTROL_MODE);
    concentrate_condition_done = false;
    decentralize_busy_flag = false;

    water_cool_init();
}

//100ms进行一次pid运算
void arg_pid_handle(void)
{

    if (_PID_CONTROL_UP_FALG == false)
        return;

    _PID_CONTROL_UP_FALG = false;

    if (have_temp_control_task() == false)
        return;

    if (get_temp_control_mode() == CONCENTRATE_CONTROL_MODE)
    {
        concentrate_control_mode_handle();
    }
    else
    {
        decentralized_control_mode_handle();
    }

    water_cool_handle();
}
