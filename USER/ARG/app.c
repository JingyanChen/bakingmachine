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

/*
 * 任务的处理框架由两个函数构成
 * queue_task_deal_handle函数负责做对应状态应当做的操作准备
 * queue_task_deal_wait_handle函数，负责等待事件，从而改变状态机
 * 两个函数切换执行，需要等待事件的时候，启动wait函数，wait 满足要求，进入切换函数
 */
bool queue_task_deal_hang_up = false;
static void queue_task_deal_init(void){
    queue_task_deal_hang_up = false;
}
static void queue_task_deal_handle(void){

    uint8_t change_water_road=0x01;

    #if defined (STOP_TEMP_ARG) 
    float stop_temp_f = 0;
    uint16_t now_temp=0;
    #endif

    if(queue_task_deal_hang_up)
        return ;

    if(get_task_machine_status() == task_machine_idle)
        return ;//无任务执行

    //根据最新的PID控制器算法核心3，仅有大范围升温任务或者有换水要求的任务会进入此状态机
    //本状态机会根据当前的任务类型来分配资源

    if(now_running_event_task.event_type == START_CONTROL_TEMP_EVENT){


        #if defined (STOP_TEMP_ARG) 
            now_temp = get_road_temp(now_running_event_task.road_id);
            stop_temp_f = ((float)now_running_event_task.target_temp - (float)now_temp) * STOP_TEMP_CAL_K + (float)now_running_event_task.target_temp;
            //尝试进入集中控温模式，有可能会失败，失败的原因是分散控温模式未达到ready状态，有几路没控制好
            if(set_pid_controller_mode_as_concentrate(now_running_event_task.road_id,now_running_event_task.target_temp,(uint16_t)stop_temp_f,STOP_TIM_DEFAULT)){
                set_temp_control_status(now_running_event_task.road_id,TEMP_CONTROL_UP_DOWN_QUICK_STATUS);   
                
                //配置换水任务
                if(now_running_event_task.need_change_water == true){
                    /*
                    * SETP1 : 处理湿度系统，换水任务，然后配置进入集中控温模式
                    */   
                    change_water_road = change_water_road << now_running_event_task.road_id ;

                    //打开换水状态机后，通过get_water_injection_status获得状态，当状态变为change_water_done时
                    //认为换水结束
                    change_water(change_water_road);

                    //配置进入等待模式
                    queue_task_deal_hang_up = true;                    
            }             
            }else{
                //进入失败，那么，本次执行任务失败，继续等待,下一次从新尝试
                queue_task_deal_hang_up = false;
                set_temp_control_status(now_running_event_task.road_id,TEMP_CONTORL_STOP);   
            }
        #else
            if(set_pid_controller_mode_as_concentrate(now_running_event_task.road_id,now_running_event_task.target_temp)){
                set_temp_control_status(TEMP_CONTROL_UP_DOWN_QUICK_STATUS);   
                //配置换水任务
                if(now_running_event_task.need_change_water == true){
                    /*
                    * SETP1 : 处理湿度系统，换水任务，然后配置进入集中控温模式
                    */   
                    change_water_road = change_water_road << now_running_event_task.road_id ;

                    //打开换水状态机后，通过get_water_injection_status获得状态，当状态变为change_water_done时
                    //认为换水结束
                    change_water(change_water_road); 

                    //配置进入等待模式
                    queue_task_deal_hang_up = true;
            }else{
                queue_task_deal_hang_up = false;
                set_temp_control_status(TEMP_CONTORL_STOP);  
            }
        #endif

    }
}

static void queue_task_deal_wait_handle(void){
    
    if(queue_task_deal_hang_up == true)
        return ;

    switch(get_temp_control_status(now_running_event_task.road_id)){
        case TEMP_CONTORL_STOP: queue_task_deal_hang_up = false; break;
        case TEMP_CONTROL_UP_DOWN_QUICK_STATUS:
            //集中升温模式的结束的条件是，集中控制的两路有一瞬间到达目标温度
            //对于系统来说，明显的标记是 concentrate_condition_done == true 
            //系统一直查询此标志位，如果查到其为true，认为集中模式结束
            if(get_concentrate_status()== true){
                set_temp_control_mode(DECENTRALIZED_CONTROL_MODE);//切换控制方式变为分散控温模式
                set_temp_control_status(now_running_event_task.road_id,TEMP_CONTROL_CONSTANT);   //指示现在该路已经进入了分散控温模式
            }
            break;
        case TEMP_CONTROL_CONSTANT:

            //检查分散控温模式是否处于ready状态，如果出于ready状态，检查湿度系统是否完成工作，
            //两个条件如果都满足了，那就意味着可以放行，执行下一个任务了，否则需要继续等待。
            if(get_decentralize_busy_flag()== false &&
                get_water_injection_status()!=change_water_out_status &&
                get_water_injection_status()!=change_water_in_status){
                    //经历过了集中控温模式，并且分散控温模式的条件也达到了，所以可以执行下一个大温度队列任务
                    //分散控温模式需要给他一段时间补偿之前因为某个集中控温模式而丢下来的温度
                    //分散控温模式如果给出busy==false的结论，说明此时所有打开的温度控制器都是处于可控状态
                    //这样可以避免第一路集中立刻第二路集中立刻第三路集中第四路集中，不间断的集中任务，导致第一路温度下滑太多
                    set_temp_control_status(now_running_event_task.road_id,TEMP_CONTROL_ALL_READY);   //不仅你需要控制的那一路到温，系统也已经补偿完了由集中模式带来的温度损失
                    //这种状态下可以接收新的任务
                }
            break;
        case TEMP_CONTROL_ALL_READY:
                //进入此状态后，可以执行下一个任务
                now_running_event_task.task_running_over = true;
                queue_task_handle();
                queue_task_deal_hang_up = false;
                queue_task_deal_handle();

            break; 
    }
}

/*
 * 消耗很小的小型任务，比如降温任务，或者小范围的升温任务
 * 将会使得road_status进入 SPECIAL_WAIT模式
 * 此模式会注册温度控制任务到分散控温状态
 * 不会抢占当前集中控温状态，因为集中控温状态一定会结束。
 * 所以会有一个handle监控是否有SPEACIAL_WAIT状态任务，有的话检查其是否到温，到温切换其为ALL_READY状态
 * 
 */
static void special_wait_status_server_handle(void){
    uint8_t i =0;
    int16_t error_temp=0;

    for(i=0;i<5;i++){
        if(get_temp_control_status(i) == TEMP_CONTROL_SPECIAL_WAIT){
            //辅助判断一下是否到温
            error_temp = get_target_temp(i) - get_road_temp(i);
            if(error_temp > -10 && error_temp < 10){
                set_temp_control_status(i,TEMP_CONTROL_ALL_READY);
            }
        }
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
	queue_task_deal_init();
}

static void arg_temp_control_handle(void)
{
    queue_task_handle();
    queue_task_deal_handle();
    queue_task_deal_wait_handle();

    special_wait_status_server_handle();
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
