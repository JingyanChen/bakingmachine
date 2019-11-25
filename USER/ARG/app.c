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

    switch (box_status[box_id])
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
        box_status[box_id] = box_running_backward;
        //如果成功的运行完成，在handle中一直等待结束条件
        //并且切换status
        break;
    case box_running_forward:
            start_motor_acc_arg_return(box_id);
            box_status[box_id] = box_running_backward;
            break;
    case box_running_backward:
            debug_sender_str("key_happend \r\n");
            start_motor_acc_arg_return(box_id);
            box_status[box_id] = box_running_forward;
            debug_sender_str("key_happend end1\r\n");
            break;
    case box_unknown:
        break;

    default:
        break;
    }

    debug_sender_str("key_happend \r\n");
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
                get_motor_limit_v(5 + i) == MOTOR_LIMIT_V)
            {
                manual_push_ready[i] = true;
                is_wait_ready_status[i] = true;
            }
        }
        else
        {
            if (box_status[i] == box_on &&
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
                get_motor_status(i) == no_running )           //之前状态是关闭状态
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
            }

            if (box_status[i] == box_running_backward && //当前状态是处于向后运动的状态
                get_motor_status(i) == no_running)           //之前状态是打开状态
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
            }

        }
    }
}

//温控委托框架下的实现队列实体以及访问方法enqueue & dequeue
static event_t event_queue[MAX_EVENT_QUEUE_DEPTH + 1];
static uint16_t queue_front=0;
static uint16_t queue_rear =0;

static bool is_queue_full(void){
    return (queue_rear + 1) % (MAX_EVENT_QUEUE_DEPTH + 1) == queue_front;
}
static bool is_queue_empty(void){
    return queue_rear == queue_front;
}
bool enqueue_event(event_t e){

    if(is_queue_full())
        return false;
    
    if (is_queue_empty()){
        event_queue[queue_front].event_type = e.event_type;
        event_queue[queue_front].road_id = e.road_id;
        event_queue[queue_front].target_temp = e.target_temp;
        event_queue[queue_front].need_change_water = e.need_change_water;        
    }

    event_queue[queue_rear].event_type = e.event_type;
    event_queue[queue_rear].road_id = e.road_id;
    event_queue[queue_rear].target_temp = e.target_temp;
    event_queue[queue_rear].need_change_water = e.need_change_water; 

    queue_rear = (queue_rear + 1) % (MAX_EVENT_QUEUE_DEPTH + 1);

    return true;
}
bool dequeue_event(event_t * e){

    if(is_queue_empty())
        return false;

    e->event_type = event_queue[queue_front].event_type;
    e->road_id = event_queue[queue_front].road_id;
    e->target_temp = event_queue[queue_front].target_temp;
    e->need_change_water = event_queue[queue_front].need_change_water;

    queue_front = (queue_front + 1) % (MAX_EVENT_QUEUE_DEPTH + 1);

    return true;

}
event_t get_front_queue_ele(void){
    event_t event_rlt;

    memcpy((uint8_t *)&event_rlt,(uint8_t *)&event_queue[queue_front],sizeof(event_t));

    return event_rlt;
}

event_t get_pos_queue_ele(uint8_t pos){
    event_t event_rlt;

    memcpy((uint8_t *)&event_rlt,(uint8_t *)&event_queue[(queue_front+pos)%(MAX_EVENT_QUEUE_DEPTH+1)],sizeof(event_t));

    return event_rlt;    
}
uint16_t get_queue_size(void){
    return (queue_rear - queue_front);
}
//end


//温控委托框架下的状态机系统
static event_t now_running_event_task;
static temp_control_status_t temp_control_status[TEMP_CONTROL_NUM];
static task_machine_status_t task_machine_status;
static void init_temp_control_status(void){
    uint8_t i=0;
    for(i=0;i<TEMP_CONTROL_NUM;i++){
        temp_control_status[i] = TEMP_CONTORL_STOP;
    }
    memset((uint8_t *)&now_running_event_task,0,sizeof(event_t));
    now_running_event_task.task_running_over = true;

    task_machine_status = task_machine_idle;
}
void set_temp_control_status(uint8_t road_id ,temp_control_status_t status){
    temp_control_status[road_id % TEMP_CONTROL_NUM] = status;
}
temp_control_status_t get_temp_control_status(uint8_t road_id){
    return temp_control_status[road_id % TEMP_CONTROL_NUM];
}
void set_now_running_event_task(event_t e){
    memcpy((uint8_t *)&now_running_event_task,(uint8_t *)&e,sizeof(event_t));
}
event_t get_now_running_event_task(void){
    event_t e;
    memcpy((uint8_t *)&e,(uint8_t *)&now_running_event_task,sizeof(event_t));
    return e;
}
void set_task_machine_status(task_machine_status_t status){
    task_machine_status = status;
}

task_machine_status_t get_task_machine_status(void){
    return task_machine_status;
}

void queue_task_handle(void){
    /*
     * 检查队列中是否有元素，如果有，那么就将其推送到正在执行的变量里
     * 等待外部给予任务结束标志
     */
    event_t e;
    if(now_running_event_task.task_running_over==true){
        //上一个任务已经执行完成
        if(get_queue_size() !=0 ){
            //队列非空
            if(dequeue_event(&e)){
                //成功获取到了一个队首任务
                set_now_running_event_task(e);

                //设置状态机为激活状态
                set_task_machine_status(task_machine_running);
            }
        }else{
            //最后一个任务结束，并且任务队列空，认为所有任务执行完毕
            set_task_machine_status(task_machine_idle);
        }
    }
}

static void queue_task_deal_handle(void){

    if(get_task_machine_status() == task_machine_idle)
        return ;//无任务执行

    //根据最新的PID控制器算法核心3，仅有大范围升温任务或者有换水要求的任务会进入此状态机器
    //本状态机会根据当前的任务类型来分配资源
    if(now_running_event_task.event_type == START_CONTROL_TEMP_EVENT){

    }

}


//end温控委托框架下的状态机系统

//温控算法
static void arg_temp_control_init(void)
{
    //清空队列状态机
    queue_front = 0;
    queue_rear = 0;

    //
	init_temp_control_status();
}

static void arg_temp_control_handle(void)
{
    queue_task_handle();
    queue_task_deal_handle();
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
