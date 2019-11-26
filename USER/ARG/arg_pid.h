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

#define STOP_TEMP_ARG //打开核心算法2

//暂停温度设置为 目标温度 (target_temp - now_temp) * STOP_TEMP_CAL_K +  target_temp
//假设由30 ->80 则升温到65°会停温，暂时设置为0.7
#define STOP_TEMP_CAL_K 0.7

#define STOP_TIM_DEFAULT 10 //暂停时间设为10S
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


/*
 * PID控制器针对应用的核心算法3
 * Author : Jingyan Chen @ ComeGene 2019.11.25
 * 
 * 目前停止操作不再与退水操作有任何联系，统一的退水操作委托给关机时统一去做
 * 且停止操作不会进入顺序任务队列，而是做如下两件事
 * 1 立刻做出响应关闭对应的温控SW
 * 2 检查当前的温控任务是否是相同的road_id 如果相同，立刻结束当前的温控任务
 * 
 *
 * 温度控制操作有三种可能
 * 
 * 1 大差距升温，一般的过程是 是否换水->集中升温->停温算法->分散控温->结束
 * 2 小差距升温，一般过程是 是否换水->分散控温->结束
 * 3 降温, 一般过程是 是否换水->打开降温阀->分散控温->结束
 * 
 * 值得思考的是 
 * 1 大温差升温需要加入停温算法 
 * 2 小温差升温 不需要停温算法
 * 3 降温不需要进入集中升温过程，因为降温不需要访问紧俏的温度系统
 * 
 * 于是 得出结论，只有大差距升温条件才会顺序执行，这里的温差可以暂时设置为
 * 30摄氏度，如果需要升的温度大于30摄氏度，认为是大温差升温。
 * 
 * 综合考虑，仅区分 升温/降温
 * 
 * 升温任务，一定是顺序执行
 * 降温任务，如果不需要换水，不进入任务队列，如果需要换水，则进入队列，等待湿度释放资源。
 * 
 * 大范围升温算法：
 *       换水->集中升温->停温算法->分散控温->结束
 * 小范围升温算法 只要超过SMALL_RANGE_UP_TEMP度，就会进入集中控温，要不然
 *      只会依靠分散控温来小范围升温。
 * 
 * 这样操作，简化流程，提升效率，更改SMALL_RANGE_UP_TEMP来区分大范围/小范围的界限
 */
/*
 * 大范围升温定义参数：
 *      系统认为，如果需要升的温度小于SMALL_RANGE_UP_TEMP/10 摄氏度，那么不需要进入队列，从而
 *      完成集中升温-分散控温这些复杂的流程，而是直接由分散模式处理。
 */
#define SMALL_RANGE_UP_TEMP 200
/*
 * 大范围降温定义参数
 *      系统认为，当需要降的温度大于SMALL_RANGE_DOWN_TEMP/10 摄氏度时，需要委托水冷系统处理降温
 *      任务。
 */
#define SMALL_RANGE_DOWN_TEMP 200
/*
 * 提前关闭水冷参数
 *      系统认为，当当前温度到达目标温度+WATER_COOL_TEMP_OFFSET时会停下
 */
#define WATER_COOL_TEMP_OFFSET 500



/*
 * PID控制器针对应用的核心算法4
 * Author : Jingyan Chen @ ComeGene 2019.11.26
 * 
 * 适度系统与集中控温模式应当同时执行，取消换水状态
 * 是否换水仅体现在开始做任务的时候是否打开换水状态机
 * 要注意，结束当前的任务之前，要判断换水状态机是否结束，以避免温度控制时间过段
 * 使得换水操作还没结束，就开始下一个任务。
 * 
 * 外部获得
 * TEMP_CONTROL_CONSTANT状态
 * 说明你需要控制的那一路已经到温度了，可以开始倒计时
 * 
 * 外部获得
 * TEMP_CONTROL_ALL_READY
 * 说明不仅你需要控制的那一路到温，系统也已经补偿完了由集中模式带来的温度损失
 * 
 */

#define PID_CONTORLLER_NUM 10


typedef enum{
    DECENTRALIZED_CONTROL_MODE=0,
    CONCENTRATE_CONTROL_MODE,
}temp_control_mode_t;



#define P 3
#define D -5.0

void arg_pid_init(void);
void arg_pid_handle(void);


bool get_pid_con_sw(uint8_t id);
void set_pid_con_sw(uint8_t id , bool sw);
int16_t get_target_temp(uint8_t road_id);

//设置/读取控制模式，集中/分散
void set_temp_control_mode(temp_control_mode_t mode);
temp_control_mode_t get_temp_control_mode(void);

/*
 * brief : 快速工具函数，获得0-4路的平均温度
 * return : temp
 */
uint16_t get_road_temp(uint8_t road_id);
/*
 * brief : 无条件关闭road_id的温控开关，执行此函数时，温控一定会处于分散控制
 *         模式，所以关闭了温控开关，就等于取消注册了分散温控模式
 */
void no_reason_stop_temp_control(uint8_t road_id);
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
 * brief : 注册pid控制器分散温控器温度事件，打开温控sw，配置目标温度
 *          注意，此函数仅仅是将此id的温度控制器打开
 *         这个函数给小范围升温用，他不会改变状态机的控制模式
 *         也就是说，不会抢占集中控温模式的时间，小任务可以放心使用
 * pra @ id : 0-4 为了简化程序，把0-1设为一对 2-3 设为一对
 * pra @ target:每一层的目标温度
 * Note: 当有升温温控需求，注册此函数是必要的，因为在集中控制之后，回到分散模式
 *       系统需要知道哪些路需要控制。
 *       这个函数仅仅注册了分散控温模式需要控温的路而已。
 */
void set_pid_controller_mode_as_decentralize_without_set_mode(uint8_t id, uint16_t target_temp);


#if defined (STOP_TEMP_ARG)
/*
 * brief : 配置集中温度控制器参数,带停温算法
 * pra @ id : 0-4 为了简化程序，把0-1设为一对 2-3 设为一对
 * pra @ target:每一层的目标温度
 * pra @ stop_temp : 停止温度 保留一位小数 250-1000
 * pra @ stop_tim : 暂停时间 单位是 S
 * return 开始是否成功
 * Note : 当有升温需求后，首先进入concentrate模式，集中升温，然后回到分散模式
 */
bool set_pid_controller_mode_as_concentrate(uint8_t road_id, uint16_t target_temp,uint16_t stop_temp,uint16_t stop_tim);
#else
/*
 * brief : 配置集中温度控制器参数
 * pra @ id : 0-4 为了简化程序，把0-1设为一对 2-3 设为一对
 * pra @ target:每一层的目标温度
 * return 开始是否成功
 * Note : 当有升温需求后，首先进入concentrate模式，集中升温，然后回到分散模式
 */
bool set_pid_controller_mode_as_concentrate(uint8_t road_id,uint16_t target_temp) ;
#endif
/*
 * brief : 委托一个水冷降温任务，状态机会打开水冷泵，一直到温度低于target_temp，关闭泵
 * pra @ id : 0-4 为了简化程序，把0-1设为一对 2-3 设为一对
 * pra @ target:每一层的目标温度
 */
void start_water_cool(uint8_t road_id ,uint16_t target_temp);
/*
 * brief : 关闭水冷委托事件
 * pra @ id : 0-4 为了简化程序，把0-1设为一对 2-3 设为一对
 */
void stop_water_cool(uint8_t road_id);
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
