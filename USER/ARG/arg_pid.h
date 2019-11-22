#ifndef __ARG_PID__
#define __ARG_PID__

#include <stdint.h>
#include <stdbool.h>


/*
 * 本驱动旨在提供一个PD控制器驱动，在handle中，PD控制器会一直获取不断被更新的温度值
 * 求出error之后，会将error通过PD算法换算为0-1000的PWM占空比数值，通过加热片反馈给温度
 * pid算法仅在升温的时候有效，降温的时候做一些动作来快速降温
 * 
 * 
 * 19.10.25
 * 
 * 决定加入自动的水冷触发系统
 * 当目标温度小于当前温度 10摄氏度的时候，触发对应的水冷泵，快速抽水降温
 * 
 * 19.11.21
 * 
 * 为适应最新的温控框架，提出PID控制器如下两种控制方式
 * 
 * 1 分散恒温控制方式：会把所有打开的温度控制路按照谁离目标温度最远，谁享受温控资源的原则
 *   分配温控资源
 * 2 集中升温温控方式：仅仅会对road进行控温，接收0-4的输入，这一段时间，温控资源
 *   只会集中使用在road[x]路，而不会补偿其他路，直到集中升温的温度瞬间到达目标温度
 *   停止此状态，并且向上级指示工作状态。
 * 
 * 为使用最新的温控算法，提出再hanle中增补监控是否需要打开降温泵的功能，
 * 条件是当前温度与目标温度差值大于某值，打开水泵直到此条件不成立。
 * 目标温度-当前温度 < - 20 && 温控开关打开 
 * 打开水泵
 * 
 */


/*
 * PID控制器针对应用的核心算法1
 * Author: Jingyan Chen @ ComeGene 2019.11.21
 * 
 * 总体功能：对10路加热片进行有温度传感器反馈的温度控制，其中十路每两路为一对
 * 进行优先快速升温，后平均分配热资源的应用算法。
 * 
 * 外部访问此状态机的开始是调用
 *  set_pid_controller_mode_as_concentrate函数，输入需要打开的层数ID，并且输入温度
 * 此函数会返回，操作是否成功，由decentralize_busy_flag来决定，
 * 如果busy_flag为true，说明上一次的恒温操作没有结束，不允许开始新的集中温控任务
 * 否则，系统会把温控信息记录给分散温度模式，但是由集中温度模式开始，将
 * concentrate_condition_done标志位复位，set_temp_control_mode开始集中模式
 * 
 * 如果set_pid_controller_mode_as_concentrate返回true，那么集中温控模式就会开始
 * 但是集中温控模式不是一个长时间保持的模式，而是一个中间阶段
 * 当集中控温的效果达到“控制的两路温度有一瞬间超过目标温度”这一条件的时候，
 * concentrate_condition_done会置位，告诉上层调用者这边集中控温模式已经结束了
 * 自动进入分散控温模式
 * 
 * 调用者发现concentrate_condition_done变为true之后，应当主动的将温控模式转化为
 * 分散控温模式使用set_temp_control_mode函数。并且对应的应当将
 * decentralize_busy_flag 置为TRUE，告诉其他调用者，这一段时间不接受新的
 * 集中控温任务，必须执行一段时间分散控温模式，直到
 * “所有被控温的路都达到目标温度+-1”的程度，此时才把BUSY位置0，允许集中温度访问
 * 
 * 以五路算法为例
 * 
 * 打开第一路控温，调用set_pid_controller_mode_as_concentrate函数
 * 一段时间内，集中升温，直到到达温度，进入分散控温模式一段时间，直到第一路温度回到
 * 控温水平，允许新的控温任务开始。
 * 
 * 仅在第一路集中升温后，进入分散控温模式，且达到分散控温模式的要求后，才允许第二路
 * 控温操作被响应。
 * 
 * 以此类推。
 * 
 * 水泵的控制操作，应由调用者进行判断，当两次温度控制指令出现，且两次的目标温度出现
 * 大范围的降温时，打开水泵直降温到目标温度，关闭水泵。
 * start_water_cool函数提供给调用者访问单一路水冷功能
 * 
 * 
 */


/*
 * PID控制器针对应用的核心算法2
 * Author : Jingyan Chen @ ComeGene 2019.11.22
 * 
 * 实验发现，停止加热的时候，板子的热均匀性更好，所以为了减少板子两边与中间的温度差，会在加热到某个
 * 温度的时候停止输出一段时间，这里存在两个参数
 * 1 STOP_T 停温温度，当传感器检测到停止温度后，触发停止输出
 * 2 STOP_PID_TIM 停温时间，当触发停温温度后，需要保持无输出的时间
 * 
 * 这个算法将会嵌入到集中控温模式中，在新版本中体现此算法。
 */
#define PID_CONTORLLER_NUM 10
//#define DEBUG_PID_SW 

typedef enum{
    DECENTRALIZED_CONTROL_MODE=0,
    CONCENTRATE_CONTROL_MODE,
}temp_control_mode_t;



#define P 35.0
#define D -5.0

void arg_pid_init(void);
void arg_pid_handle(void);


bool get_pid_con_sw(uint8_t id);
void set_pid_con_sw(uint8_t id , bool sw);

/*
 * brief : 注册pid控制器分散温控器温度事件，打开温控sw，配置目标温度
 *          注意，此函数仅仅是将此id的温度控制器打开
 * pra @ id : 0-4 为了简化程序，把0-1设为一对 2-3 设为一对
 * pra @ target:每一层的目标温度
 * Note: 当有升温温控需求，注册此函数是必要的，因为在集中控制之后，回到分散模式
 *       系统需要知道哪些路需要控制。
 */
void set_pid_controller_mode_as_decentralize(uint8_t id , uint16_t target_temp);

/*
 * brief : 配置集中温度控制器参数
 * pra @ id : 0-4 为了简化程序，把0-1设为一对 2-3 设为一对
 * pra @ target:每一层的目标温度
 * return 开始是否成功
 * Note : 当有升温需求后，首先进入concentrate模式，集中升温，然后回到分散模式
 */
bool set_pid_controller_mode_as_concentrate(uint8_t road_id,uint16_t target_temp) ;

/*
 * brief : 委托一个水冷降温任务，状态机会打开水冷泵，一直到温度低于target_temp，关闭泵
 * pra @ id : 0-4 为了简化程序，把0-1设为一对 2-3 设为一对
 * pra @ target:每一层的目标温度
 * return 开始是否成功
 */
void start_water_cool(uint8_t road_id ,uint16_t target_temp);

/*
 * concentrate_condition_done 指示集中控温模式是否结束
 * true为结束，false为未结束，结束后自动切换到分散控温模式
 */
bool get_concentrate_status(void);

/*
 * decentralize_busy_flag 指示集中模式结束后进入分散模式，分散模式
 * 是否具备开始新的集中模式的条件？
 * 如果为true说明不具备，因为前几个打开的温度离目标温度差别太大，不应该开始
 * 一个新的集中模式。
 * 如果为FALSE代表具备，前几个打开温度控制已经处于一个合适的工作状况，可以开始
 * 一个新的集中模式。
 */
bool get_decentralize_busy_flag(void);

#endif
