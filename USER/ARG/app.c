#include "app.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "csp_timer.h"
#include "csp_gpio.h"
#include "csp_uart.h"

#include "periph_motor.h"
#include "periph_key.h"
#include "periph_humidity_sys.h"

#include "arg_pid.h"

//APP TASK 1
/*
 * 推出/推入盒子，配合对应的按键做动作
 * 目前的逻辑：
 * 默认是灭，按下之后，电容按键灯亮起，开始做正向方向的加减速运动
 * 在这个过程中，如果电容按键再次被按下，以一个比较柔和的速度退回
 * 如果在退回的过程中又发生按下的事件，不再予以响应，直等到退回完成，根据目前的电平状态做对应动作。
 * 
 */

/*
 * box_status是当前的运动状态
 * box_pri_status是上一个状态
 */
box_status_t box_status[BOX_NUM] = {box_unknown, box_unknown, box_unknown, box_unknown, box_unknown};
box_status_t box_pri_status[BOX_NUM] = {box_unknown, box_unknown, box_unknown, box_unknown, box_unknown};

box_status_t get_box_status(uint8_t box_id)
{
    return box_status[box_id % 5];
}

bool arg_box_push_pop_init_done = false;
static bool check_box_restart_ok(void)
{
    uint8_t i = 0;
    for (i = 0; i < BOX_NUM; i++)
    {
        if (get_motor_status(i) == is_running)
        {
            return false;
        }
    }

    return true;
}

//考虑匀速退回的时候按下是否可以迅速弹出
void key_box_logic(uint8_t box_id)
{
    //出现了box id 的按键事件
    //根据上一次的状态决定接下来的动作

    switch (box_pri_status[box_id])
    {
    case box_off:
        //设备处于关闭状态，按下之后做向前加减速运动操作
        start_motor_acc_arg(box_id, BOX_FORWARD_DIR, RUN_TIM_ALL_ROUTE);
        //改变目前的状态box_status为forward
        box_status[box_id] = box_running_forward;
        //如果成功的运行完成，在handle中一直等待结束条件
        //并且切换status
        break;
    case box_on:
        //设备处于关闭状态，按下之后做向前加减速运动操作
        start_motor_acc_arg(box_id, BOX_BACKWARD_DIR, RUN_TIM_ALL_ROUTE);
        //改变目前的状态box_status为forward
        box_status[box_id] = box_running_forward;
        //如果成功的运行完成，在handle中一直等待结束条件
        //并且切换status
        break;

    case box_running_forward:
        //在设备处于加减速向前运动的状态下，突然反悔，向反方向运动
        start_motor_acc_arg_return(box_id);
        //开始配置状态机，做反悔运动
        box_status[box_id] = box_running_backward;
        break;
    case box_running_backward:
        //在设备处于加减速向前运动的状态下，突然反悔，向反方向运动
        start_motor_acc_arg_return(box_id);
        //开始配置状态机，做反悔运动
        box_status[box_id] = box_running_forward;    
        break;        
    case box_unknown:
        break;
    
    default : break;
    }
}
static void key_handle_0(void) { key_box_logic(0); }
static void key_handle_1(void) { key_box_logic(1); }
static void key_handle_2(void) { key_box_logic(2); }
static void key_handle_3(void) { key_box_logic(3); }
static void key_handle_4(void) { key_box_logic(4); }

static void arg_box_push_pop_init(void)
{
    uint8_t i = 0;

    //TODO 开机检查一次五个盒子是否都处于box_off 状态，通过机构的五个限位开关来感受
    //如果没有处于off状态，那么发起低速匀速退回运动

    if (BOX_BACKWARD_DIR == CW)
    {
        //退回的方向是CW,那么检查CW方向的限位开关是否是处于按下的状态
        //如果不是，发起一次向BACKWARD方向的匀速运动，直到碰到限位为止
        for (i = 0; i < 5; i++)
        {
            if (get_motor_limit_v(i) != MOTOR_LIMIT_V)
            {
                //发现没有触碰光电开关的情况
                //发起一次匀速复位运动
                set_motor_speed_dir(i, BOX_BACKWARD_DIR, 700);
            }
        }
    }
    else
    {
        for (i = 0; i < 5; i++)
        {
            if (get_motor_limit_v(5 + i) != MOTOR_LIMIT_V)
            {
                //发现没有触碰光电开关的情况
                //发起一次匀速复位运动
                set_motor_speed_dir(i, BOX_BACKWARD_DIR, 700);
            }
        }
    }

    if (check_box_restart_ok())
    {
        for (i = 0; i < BOX_NUM; i++)
        {
            box_status[i] = box_off;
        }
        arg_box_push_pop_init_done = true;

        //注册按键服务函数
        register_key_press_event(0, key_handle_0);
        register_key_press_event(1, key_handle_1);
        register_key_press_event(2, key_handle_2);
        register_key_press_event(3, key_handle_3);
        register_key_press_event(4, key_handle_4);
    }
    else
    {
        //出现盒子没有复位的情况，初始化过程托管给handle
        arg_box_push_pop_init_done = false;
    }
}
/*
 * 提供一个手推盒子，自动弹回的机制
 * 此状态机触发的要求很严格，100ms检查一次
 * 触发条件 :盒子处于on状态 终点限位开关处于按下状态 
 *             出现一个时刻，终点限位开关不处于按下状态
 * 触发内容：触发一次按键任务
 * TODO TEST TASK
 */
static bool manual_push_ready[BOX_NUM];
static bool is_wait_ready_status[BOX_NUM];

static void arg_manual_push_init(void)
{
    uint8_t i=0;
    for(i=0;i<BOX_NUM;i++){
        manual_push_ready[i] = false;
        is_wait_ready_status[i] = false;
    }
}

static void arg_manual_push_handle(void)
{
    uint8_t i = 0;
    for (i = 0; i < BOX_NUM; i++)
    {
        if (is_wait_ready_status[i] == false)
        {
            if (box_status[i] == box_on &&
                box_pri_status[i] == box_on &&
                get_motor_limit_v(5 + i) == MOTOR_LIMIT_V)
            {
                manual_push_ready[i] = true;
                is_wait_ready_status[i] = true;
            }
        }
        else
        {
            if (box_status[i] == box_on &&
                box_pri_status[i] == box_on &&
                get_motor_limit_v(5 + i) != MOTOR_LIMIT_V &&
                manual_push_ready[i] == true)
            {
                //事件被触发

                key_box_logic(i);
                manual_push_ready[i] = false;
                is_wait_ready_status[i] = false;               
            }
        }
    }
}
static void arg_box_push_pop_handle(void)
{
    uint8_t i = 0;
    if (arg_box_push_pop_init_done == false)
    {
        //初始化未成功一直保持初始化的过程
        if (check_box_restart_ok())
        {
            for (i = 0; i < BOX_NUM; i++)
            {
                box_status[i] = box_off;
                box_pri_status[i] = box_off;
            }
            arg_box_push_pop_init_done = true;

            //注册按键服务函数
            register_key_press_event(0, key_handle_0);
            register_key_press_event(1, key_handle_1);
            register_key_press_event(2, key_handle_2);
            register_key_press_event(3, key_handle_3);
            register_key_press_event(4, key_handle_4);
        }
        else
        {
            //出现盒子没有复位的情况，初始化过程托管给handle
            arg_box_push_pop_init_done = false;
        }
    }
    else
    {
        //初始化成功，接收按键对电机运动的访问

        //反复查询中间状态是否转变为最终状态
        for (i = 0; i < BOX_NUM; i++)
        {
            if (box_status[i] == box_running_forward && //当前状态是处于向前运动的状态
                get_motor_status(i) == no_running &&    //电机停止运动，因为碰到限位开关
                box_pri_status[i] == box_off)           //之前状态是关闭状态
            {
                /*
                     * 满足了三个条件
                     * 1 当前的运动状态是向前运动
                     * 2 目前电机已经停止运动了
                     * 3 在发起运动之前，盒子处于关闭状态
                     * 
                     * 初始化之后第一次电机电容按键，中途没有反悔，让状态机完整运行完，一定是进入这个状态
                     */
                box_status[i] = box_on;
                box_pri_status[i] = box_on;
            }
            if (box_status[i] == box_running_backward && //当前状态是处于向后运动的状态
                get_motor_status(i) == no_running &&    //电机停止运动，因为碰到限位开关
                box_pri_status[i] == box_on)           //之前状态是打开状态
            {
                /*
                     * 满足了三个条件
                     * 1 当前的运动状态是向前运动
                     * 2 目前电机已经停止运动了
                     * 3 在发起运动之前，盒子处于关闭状态
                     * 
                     * 初始化之后第一次电机电容按键，中途没有反悔，让状态机完整运行完，进入off状态后
                     * 再点击一次按键，不打断其运动，顺利复位，会进入此状态。
                     * 
                     */
                box_status[i] = box_off;
                box_pri_status[i] = box_off;
            }
            if (box_status[i] == box_running_backward && //当前状态是处于向后运动的状态
                get_motor_status(i) == no_running &&    //电机停止运动，因为碰到限位开关
                box_pri_status[i] == box_running_forward)           //之前状态是前进状态
            {
                /*
                     * 满足了三个条件
                     * 1 当前的运动状态是向前运动
                     * 2 目前电机已经停止运动了
                     * 3 之前是前进状态，后来反悔了变成了后退状态
                     * 
                     * 初始化之后第一次电机电容按键，中途反悔了，电机还没有走到零点的位置又按下了按键
                     * 这时候是后悔回退操作，不被打断最终的状态时box_off
                     */
                box_status[i] = box_off;
                box_pri_status[i] = box_off;
            }

            if (box_status[i] ==  box_running_forward && //当前状态是处于向前运动的状态
                get_motor_status(i) == no_running &&    //电机停止运动，因为碰到限位开关
                box_pri_status[i] == box_running_backward)           //之前状态是后退状态
            {
                /*
                     * 满足了三个条件
                     * 1 当前的运动状态是向前运动
                     * 2 目前电机已经停止运动了
                     * 3 之前是前进状态，后来反悔了变成了前进状态
                     * 
                     * 初始化之后第一次电机电容按键，中途反悔了，电机还没有走到零点的位置又按下了按键
                     * 这时候是后悔回退操作，不被打断最终的状态时box_off
                     */
                box_status[i] = box_on;
                box_pri_status[i] = box_on;
            }

        }
    }
}

temp_control_t tc;
static bool arg_temp_config_done = false;

//提供给与TFT屏交互的通讯代码，访问整体的温控/湿控模块功能
//输入参数填满 temp_contorl_t 类型
error_t config_temp_control_machine(temp_control_t *temp_control)
{

    uint8_t i = 0;

    if (temp_control->control_num == 0)
        return pid_temp_control_pra_error;

    //打开换水进程
    //可以考虑集体换水，给所有路换水

    if (temp_control->need_change_water == true)
    {
        change_water(0xff);
    }

    //将几路温度保存，不使用的PID控制器完全关闭
    //也就是说此函数也是关闭函数

    //赋值给全局变量tc

    tc.control_num = temp_control->control_num;
    tc.need_change_water = temp_control->need_change_water;
    memcpy(tc.control_sw, temp_control->control_sw, 10);
    memcpy(tc.control_temp, temp_control->control_temp, 10);

    //关闭没用到的PID控制器

    for (i = 0; i < CONTROL_TEMP_NUM; i++)
    {
        if (tc.control_sw[i] == false)
        {
            set_pid_con_sw(i, false);
        }
    }

    arg_temp_config_done = false;

    return no_error;
}

void close_all_temp_controller(void)
{
    temp_control_t tempctl;

    tempctl.control_num = 0;
    memset(tempctl.control_sw, false, 10);
    memset(tempctl.control_temp, 0xffff, 10);
}

temp_control_t get_temp_control_machine_status(void)
{
    return tc;
}

//温度控制算法，自动水阀控制，控温之前自动加湿状态机
static void arg_temp_control_init(void)
{
    close_all_temp_controller();
    arg_temp_config_done = true;
}

static void arg_temp_control_handle(void)
{

    uint8_t i = 0;

    if (arg_temp_config_done == true)
        return;

    //判断换水是否完成，如果完成了则可以开始所有pid控温
    //先判断有没有换水控温的任务

    for (i = 0; i < 10; i++)
    {
        if (tc.control_sw[i] == true)
            //至少有一路换水控温的任务
            goto VAILD_TASK_PRA;
    }

    return;

VAILD_TASK_PRA:

    //判断换水是否完成
    if (tc.need_change_water == true)
    {
        if (get_water_injection_status() == change_water_done)
        {

            //换水完毕
            set_water_injection_status(no_injection_task);
            //初始化一下，使得此if仅进入一次
            //if 进入一次的好处在于，条件成立的次数与调用periph_water_injection一致

            //配置一次PID控制器

            for (i = 0; i < 10; i++)
            {
                if (tc.control_sw[i] == true)
                    start_pid_controller_as_target_temp(i, tc.control_temp[i]);
            }

            arg_temp_config_done = true;
        }
    }
    else
    {
        //直接更新配置一次PID控制器

        for (i = 0; i < 10; i++)
        {
            if (tc.control_sw[i] == true)
                start_pid_controller_as_target_temp(i, tc.control_temp[i]);
        }

        arg_temp_config_done = true;
    }
}

void arg_app_init(void)
{
    arg_box_push_pop_init();
    arg_manual_push_init();
    arg_temp_control_init();
}

void arg_app_hanlde(void)
{
    if (_APP_UPDATE_FLAG == false)
        return;

    _APP_UPDATE_FLAG = false;

    arg_box_push_pop_handle();
    arg_manual_push_handle();
    arg_temp_control_handle();    
}
