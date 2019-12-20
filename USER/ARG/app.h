#ifndef _APP_
#define _APP_

#include "periph_motor.h"
#include <stdbool.h>
/*
 * 
 * 本驱动提供顶级的APP软件层支持
 * 
 * 包含如下功能
 *      1 按键弹出盒子/按键收回盒子 任务
 *      2 温度控制算法任务 包括温度控制，控温之前换水，自动切换水冷泵
 * 
 *      
 */

#define BOX_NUM 5

typedef enum{
    box_off=0,
    box_running_forward,
    box_running_backward,
    box_on,
    box_unknown,//开机的时候还没有做完复位操作后的状态
}box_status_t; 



#define BOX_FORWARD_DIR   CW
#define BOX_BACKWARD_DIR  CCW
#define BOX_RUNNING_TIM 2800

// 0 - 4 阻止 CW 方向
// 5 - 9 阻止 CCW 方向

//获得盒子的当前状态 
box_status_t  get_box_status(uint8_t box_id);
//通过此函数，访问盒子运动功能，模拟按下按键
void key_box_logic(uint8_t box_id);

/*
 * 温控算法新原则：基于委托的温控方式
 * 
 * 提供给外部的API函数有所改变，由一次性配置完所有需要操作的温控信息，改为队列形式
 * 进队和出队的形式，进入温控事件元素
 * 温控事件元素的内容包括
 * 1 这是第几路操作
 * 2 控温目标温度
 * 3 是否需要换水
 * 
 * 每一路包含五种状态
 * 
 * 未启动状态：每次开机后默认回到此状态
 * 换水状态：是否需要换水，换水的话就会存在此中间态
 * 升温状态：快速升温状态
 * 控温状态：控温状态，此状态是结束状态
 * 等待状态：等待其他路快速升温，此时因为没有获得温度资源，温度会掉
 * 
 * 委托原则：
 * 
 * 如果有控温请求，那么会压入待处理事件队列，定时检查待处理队列里是否有需要处理的事件
 * 如果有，占用状态机，直到其进入控温状态，切换下一个任务。停止指令比较特殊，可以再
 * 任何状态下执行，但是打开其他路控温指令是没有这个特权的。
 * 
 * 接收的事件类型有两个
 * 1 打开x路控温
 * 2 关闭x路控温
 * 
 * 将最多50个事件压入队列，然后状态机顺序执行
 * 
 * 温度资源分享原则：适用于升温的过程原则
 * 
 * 条件1 出现了2个以上的温控命令
 * 条件2 第三个温控命令是升温指令，抢夺恒温资源
 * 
 * 于是
 * 
 * 将前两个或前三个或前四个均进入一次恒温态，后再响应第三个/第四个/第五个温度控制指令
 */

//温控委托框架下的实现队列实体以及访问方法enqueue & dequeue

#define MAX_EVENT_QUEUE_DEPTH 50

typedef enum{
    START_CONTROL_TEMP_EVENT=0,
    STOP_CONTROL_TEMP_EVEN,
}user_event_type_t;

typedef enum{
    task_machine_idle=0,
    task_machine_running,
}task_machine_status_t;

typedef struct{
    user_event_type_t event_type;
    uint8_t road_id;
    uint16_t target_temp;
    bool need_change_water;
    bool task_running_over; //创建此对象的时候，必须初始化其为FALSE
}event_t;

/*
 * brief : enqueue event into queue 
 * pra @ e : enqueue element
 * return operate is success or queue full
 */
bool enqueue_event(event_t e);

/*
 * brief : dequeue event form queue
 * pra @e : dequeue element pointer
 * return operate is success or queue empty
 */
bool dequeue_event(event_t * e);

/*
 * brief : get event form queue without dequeue
 * return first element of queue
 */
event_t get_front_queue_ele(void);

/*
 * brief : get postion element 
 * Note :谨慎使用此函数，有可能回复的并不是队列里的数据而是空数据
 *       用此函数之前应当查询队列的总长度
 *       此函数逻辑不严谨，仅允许用在debug作用
 */
event_t get_pos_queue_ele(uint8_t pos);

/*
 * brief : get queue size
 * return queu size
 */
uint16_t get_queue_size(void);

//#end 温控委托框架下的实现队列实体以及访问方法enqueue & dequeue

//获得重要的BOOL值
bool get_queue_task_deal_hang_up(void);


//温控委托框架下的状态机系统

#define TEMP_CONTROL_NUM 5

/*
 * 下述状态会通过常规的读温度指令指示每一路的运行状态
 * TEMP_CONTORL_STOP：停止状态，此路未做任何控温/换水操作
 * TEMP_CONTROL_UP_DOWN_QUICK_STATUS：此路在执行集中升温控制
 * TEMP_CONTROL_CONSTANT：此路正在分散控温，并且温度已经到达指定温度
 * TEMP_CONTROL_ALL_READY: 不仅此路温度控制完成，而且所有正在控制的问题都是处于可控状态
 * TEMP_CONTROL_SPECIAL_WAIT:适用于不进入任务队列的温度控制路，此状态可以被集中任务打断
 *                           但是不能打断集中任务 ,专门会有一个handle为其服务。
 *                          换言之 如果当前没有集中升温任务，小任务会快速的并行执行。
 * 集中在温度指令中得以体现
 */
typedef enum{
    TEMP_CONTORL_STOP=0,
    TEMP_CONTROL_UP_DOWN_QUICK_STATUS,
    TEMP_CONTROL_CONSTANT,
    TEMP_CONTROL_ALL_READY,
    TEMP_CONTROL_SPECIAL_WAIT,
}temp_control_status_t;


/*
 * brief : write temp control status
 * pra @ road_id : road id 
 * pra @ status : road temp_control status
 */
void set_temp_control_status(uint8_t road_id ,temp_control_status_t status);

/*
 * brief : read temp control status
 * pra @ road_id : road id 
 * return status
 */
temp_control_status_t get_temp_control_status(uint8_t road_id);


/*
 * brief : 读写存放当前被任务队列推出来的执行任务信息
 */
void set_now_running_event_task(event_t e);

/*
 * brief : 读写存放当前被任务队列推出来的执行任务信息
 */
event_t get_now_running_event_task(void);

/*
 * 只要有一个任务从队列中被推出，那么任务状态机都是被激活的状态
 * 当所有任务都完成的时候，任务状态机处于idle状态
 */
void set_task_machine_status(task_machine_status_t status);
task_machine_status_t get_task_machine_status(void);

/*
 * brief : 检查当前任务是否结束，如果结束了切换下一个任务
 */
void queue_task_handle(void);

//end温控委托框架下的状态机系统




/*
 * 2019.12.20 加入停止增补代码以修正如下bug
 * 停止未正在执行的任务，停止不了，只能停止当前正在运行的任务
 * 增补代码：如果出现了停止任务，先记录，等待到切换任务的时候查验
 * 是否历史上出现过停止任务，如果是，那么不执行相关操作，跳过。
 * 
 * 算法如下：
 *      如果出现了停止未执行任务的需求
 * 先通过set_task_stop_data记录下来需求，等dequeue_event的时候判断
 * 是否有过曾经的停止任务，如果有则丢失此dequeue数据。
 */

void set_task_stop_data(uint8_t id , bool sw);
bool get_task_stop_data(uint8_t id);


void arg_app_init(void);
void arg_app_hanlde(void);


#endif
