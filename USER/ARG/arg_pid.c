#include "arg_pid.h"
#include "app.h"
#include "csp_adc.h"
#include "csp_timer.h"
#include "csp_pwm.h"
#include "csp_uart.h"
#include "csp_gpio.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "arg_debug_pro.h"
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

void set_temp_control_mode(temp_control_mode_t mode)
{
    temp_control_mode = mode;
}

temp_control_mode_t get_temp_control_mode(void)
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

int16_t get_target_temp(uint8_t road_id){
     int16_t sum_target_temp=0;

     sum_target_temp = pid_target_temp[road_id % 10] + pid_target_temp[(road_id *2 + 1) % 10];
     sum_target_temp /= 2;

     return sum_target_temp;
}
void set_pid_controller_mode_as_decentralize(uint8_t id, uint16_t target_temp)
{

    pid_target_temp[(id) % 10] = target_temp + 5;
    pid_target_temp[(id * 2 + 1) % 10] = target_temp + 5;

    set_pid_con_sw(id % 10, true);
    set_pid_con_sw((id * 2 + 1) % 10, true);

    set_temp_control_mode(DECENTRALIZED_CONTROL_MODE);
}

void set_pid_controller_mode_as_decentralize_without_set_mode(uint8_t id, uint16_t target_temp)
{

    pid_target_temp[(id) % 10] = target_temp + 5;
    pid_target_temp[(id * 2 + 1) % 10] = target_temp + 5;

    set_pid_con_sw(id % 10, true);
    set_pid_con_sw((id * 2 + 1) % 10, true);
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

/*
 * 2019.11.26
 * 在集中控温模式中加入停温算法，因为停温算法是待验证的，是为了增加板子温度均匀性但是损失了
 * 升温效率的操作，所以本代码用宏定义的方式给予整段代码作用/不作用
 * 
 * 停温算法的两个核心参数：
 * PRA1 停温温度 STOP_TEMP ：集中升温到STOP_TEMP，会屏蔽PID输出
 * PRA2 屏蔽PID输出时间 STOP_TIM :停温时间
 * 
 */

#if defined (STOP_TEMP_ARG)
static uint16_t stop_temp_pra=0;
static uint16_t stop_tim_pra=0;
static bool stop_arg_bool_1=false;
static bool stop_arg_bool_2=false;
static uint16_t stop_arg_tick_1=0;
static uint16_t stop_arg_tick_2=0;
static bool pwm_out1_lock = false;
static bool pwm_out2_lock = false;
bool set_pid_controller_mode_as_concentrate(uint8_t road_id, uint16_t target_temp,uint16_t stop_temp,uint16_t stop_tim)
{
    if(decentralize_busy_flag == true){
        return false;
    }

    set_pid_controller_mode_as_decentralize(road_id, target_temp);
    set_temp_control_mode(CONCENTRATE_CONTROL_MODE);

    concentrate_control_mode_road_id[0] = road_id;
    concentrate_control_mode_road_id[1] = road_id * 2 + 1;

    concentrate_condition_done  = false;

    stop_temp_pra = stop_temp;
    stop_tim_pra = stop_tim;
    stop_arg_bool_1 = false;
    stop_arg_bool_2 = false;
    pwm_out1_lock = false;
    pwm_out2_lock = false;

    stop_arg_tick_1 = 0;
    stop_arg_tick_2 = 0;
    
    return true;
}
#else
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
#endif


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

    float p_cal1_f = 0;
    float d_cal1_f = 0;
    float p_cal2_f = 0;
    float d_cal2_f = 0;  

    uint8_t debug_buf[200];
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


    if (get_pid_con_sw(running_pid_id[0] % 10))
    {
        if (pid_temp_error[running_pid_id[0] % 10] > 0)
        {
            //开始计算PWM占空比，使用PD算法
            p_cal1_f = (float)P * (float)pid_temp_error[running_pid_id[0] % 10];
            d_cal1_f = (float)D * ((float)pid_temp_error_last[running_pid_id[0] % 10] - (float)pid_temp_error[running_pid_id[0]]);
            pwm_out1_f =  p_cal1_f + d_cal1_f;

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
            p_cal2_f = (float)P * (float)pid_temp_error[running_pid_id[1] % 10];
            d_cal2_f = (float)D * ((float)pid_temp_error_last[running_pid_id[1] % 10] - (float)pid_temp_error[running_pid_id[1]]);
            pwm_out2_f =  p_cal2_f + d_cal2_f;

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
                goto exit;
            } 
        }
    }

    decentralize_busy_flag = false;

    exit:
    //上报代码段
    if(get_pid_debug_sw() == true){
        sprintf((char *)debug_buf,"| decentralized Mode | ID %d (p:%.2f,d:%.2f,pwm:%d,T %d/%d) ID %d (p:%.2f,d:%.2f,pwm:%d,T %d/%d)\r\n",
                running_pid_id[0],p_cal1_f,d_cal1_f,pwm_out1,pid_now_temp[running_pid_id[0]],pid_target_temp[running_pid_id[0]],
                running_pid_id[1],p_cal2_f,d_cal2_f,pwm_out2,pid_now_temp[running_pid_id[1]],pid_target_temp[running_pid_id[1]]);
        debug_sender_str(debug_buf);
    }
}

/*
 * 集中升温模式
 * 触发此模式，必须实现配置好需要集中控制的road id号
 * 此升温模式是暂时的短暂的升温模式，只要一次到达目标温度，那么升温模式就结束了
 * 19.11.26 重新整理报文输出，通过打开PID控制器状态报文来指示
 */
void concentrate_control_mode_handle(void)
{
    float pwm_out1_f = 0;
    float pwm_out2_f = 0;
    float p_cal1_f = 0;
    float d_cal1_f = 0;
    float p_cal2_f = 0;
    float d_cal2_f = 0;    

    uint16_t pwm_out1 = 0;
    uint16_t pwm_out2 = 0;
    uint16_t id1=0;
    uint16_t id2=0;

    //上传报文使用
    uint8_t debug_buf[200];
    

    id1 = concentrate_control_mode_road_id[0] % 10;
    id2 = concentrate_control_mode_road_id[1] % 10;

    //仅更新集中两路的温度
    pid_now_temp[id1] = adc_temp_data[id1];
    pid_now_temp[id2] = adc_temp_data[id2];

    pid_temp_error[id1] = pid_target_temp[id1] - pid_now_temp[id1];
    pid_temp_error[id2] = pid_target_temp[id2] - pid_now_temp[id2];


    #if defined (STOP_TEMP_ARG)

    if(stop_arg_bool_1 == false){
        if(pid_now_temp[id1] > stop_temp_pra){
            //出现一个瞬间 温度超过停止温度
            //锁上pwm1
            //以后不再判断
            pwm_out1_lock = true;
            stop_arg_bool_1 = true;
        }
    }

    if(stop_arg_bool_2 == false){
        if(pid_now_temp[id2] > stop_temp_pra){
            pwm_out2_lock = true;
            stop_arg_bool_2 = true;
        }
    }

    //计时锁定时间 自动解锁
    if(pwm_out1_lock){
        stop_arg_tick_1 ++;
        if(stop_arg_tick_1 > stop_tim_pra * 10){
            stop_arg_tick_1 = 0;
            pwm_out1_lock = false;//unlock
        }
    }

    if(pwm_out2_lock){
        stop_arg_tick_2 ++;
        if(stop_arg_tick_2 > stop_tim_pra * 10){
            stop_arg_tick_2 = 0;
            pwm_out2_lock = false;//unlock
        }
    }
    #endif

    //开始PID控温
    if (pid_temp_error[concentrate_control_mode_road_id[0] % 10] > 0)
    {
        p_cal1_f = (float)P * (float)pid_temp_error[id1];
        d_cal1_f = (float)D * ((float)pid_temp_error_last[id1] - (float)pid_temp_error[id1]);

        pwm_out1_f = p_cal1_f + d_cal1_f;

        pid_temp_error_last[id1] = pid_temp_error[id1];

        pwm_out1 = (uint16_t)pwm_out1_f;

        if (pwm_out1 > 1000)
            pwm_out1 = 1000;

        #if defined (STOP_TEMP_ARG)
        if(pwm_out1_lock)
            set_software_pwm(id1, 0); 
        else
            set_software_pwm(id1, pwm_out1);
        #else
        set_software_pwm(id1, pwm_out1);
        #endif
    }else
    {
        pwm_out1 = 0;

        set_software_pwm(id1, pwm_out1);        
    }

    if (pid_temp_error[id2] > 0)
    {
        p_cal2_f = (float)P * (float)pid_temp_error[id2];
        d_cal2_f = (float)D * ((float)pid_temp_error_last[id2] - (float)pid_temp_error[id2]);

        pwm_out2_f = p_cal2_f + d_cal2_f;
        pid_temp_error_last[id2] = pid_temp_error[id2];

        pwm_out2 = (uint16_t)pwm_out2_f;

        if (pwm_out2 > 1000)
            pwm_out2 = 1000;

        #if defined (STOP_TEMP_ARG)
        if(pwm_out2_lock)
            set_software_pwm(id2, 0); 
        else
            set_software_pwm(id2, pwm_out2);
        #else
        set_software_pwm(id2, pwm_out2);
        #endif

    }else{

        pwm_out2 = 0;

        set_software_pwm(id2, pwm_out2);            
    }

    //TODO 目前集中升温模式的结束标志仅仅是瞬间到达目标温度
    //后期可以再考虑结束的标志，结束标志决定是否进入控温态
    if(pid_temp_error[id1] < 0 &&
       pid_temp_error[id2] < 0 ){

        //两个都到达温度了，集中温度模式结束
        concentrate_condition_done = true;
       }
    
    //报文上传代码段，将此次的信息报给上位机

    if(get_pid_debug_sw() == true){
        sprintf((char *)debug_buf,"| concentrate Mode | ID %d (p:%.2f,d:%.2f,pwm:%d,T %d/%d) ID %d (p:%.2f,d:%.2f,pwm:%d,T %d/%d)\r\n",
                id1,p_cal1_f,d_cal1_f,pwm_out1,pid_now_temp[id1],pid_target_temp[id1],
                id2,p_cal2_f,d_cal2_f,pwm_out2,pid_now_temp[id2],pid_target_temp[id2]);
        debug_sender_str(debug_buf);
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

static uint16_t water_cool_target_temp[5];
static bool water_cool_sw[5];
void stop_water_cool(uint8_t road_id){
    water_cool_pump_control(road_id % 5, 1 );//关闭水冷降温
    water_cool_target_temp[road_id % 5] = false;    
}
void start_water_cool(uint8_t road_id ,uint16_t target_temp){

    water_cool_pump_control(road_id % 5, 0 );//打开水冷降温
    water_cool_target_temp[road_id % 5] = true;

}
void water_cool_init(void){
    uint8_t i=0;
    for(i=0;i<5;i++){
        water_cool_target_temp[i] = 0;
        water_cool_sw[i] = false;
    }
}

uint16_t get_road_temp(uint8_t road_id){
    uint16_t water_cool_road_temp = 0;
    
    water_cool_road_temp += adc_temp_data[road_id % 10];
    water_cool_road_temp += adc_temp_data[(road_id * 2 + 1) % 10];    

    water_cool_road_temp /= 2;

    return water_cool_road_temp;
}
void water_cool_handle(void){
    uint16_t water_cool_road_temp = 0;
    uint8_t i=0;

    if(water_cool_sw[0] == false &&
        water_cool_sw[1] == false &&
        water_cool_sw[2] == false &&
        water_cool_sw[3] == false &&
        water_cool_sw[4] == false)
        return ;

    for(i=0;i<5;i++){
        if(water_cool_sw[i] == true){
            water_cool_road_temp += adc_temp_data[i % 10];
            water_cool_road_temp += adc_temp_data[(i * 2 + 1) % 10];

            water_cool_road_temp /= 2;

            if(water_cool_road_temp < water_cool_target_temp[i] + WATER_COOL_TEMP_OFFSET){
                water_cool_pump_control(i % 5, 1 );//关闭水冷降温
                water_cool_sw[i] = false;
            }
        }
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

    set_temp_control_mode(DECENTRALIZED_CONTROL_MODE);
    concentrate_condition_done = false;
    decentralize_busy_flag = false;

    water_cool_init();
}

void no_reason_stop_temp_control(uint8_t road_id){
    set_pid_con_sw(road_id % 10,false);
    set_pid_con_sw((road_id * 2 + 1) % 10,false);
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
