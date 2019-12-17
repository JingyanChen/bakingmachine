#ifndef __ARG_VERSION__
#define __ARG_VERSION__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>


#define MAIN_VERSION    2
#define SECOND_VERSION  4
#define IS_RELEASE      1


/*
 * VERSION LOG 
 * 
 * V0.1 基本框架
 * V0.2 完成了注水逻辑 温控方法 还剩余与上位机交互，温控APP逻辑算法
 * V0.3 完成了power代码 完成了看门狗算法，进一步优化DEBUG功能
 * V0.4 升级了MOTOR包，BOX现在功能与DVD驱动器看齐，待微调测试
 * 
 * 
 * V 1.0版本
 * 
 * 修改了温控策略，大幅度修改了PID算法以适应新的温控策略
 * 
 * PID算法控制器修改如下
 * 定义了 集中控温模式 分散控温模式 并且提供了两个bool状态 concentrate_done 和 busy
 * 模糊了所有的PID API ROAD输入，输入的是0-4的
 * 所以例如想打开第0路控温，调用集中控温API，输入0
 * 此时状态机先集中控温第0路，集中控温结束concentrate_done置位，自动进入分散控温模式
 * 当分散控温模式一样结束，会复位busy位，通知上位机。
 * 
 * 所以，打开第0路的前提下，想打开第1路温控，必须在busy复位的条件下调用，否则集中温控函数API会返回false
 * 请求失败。
 * 
 * 水冷的算法如下
 * 
 * 调用者一次只能访问一路水冷泵，水冷在集中控温模式的时间域被访问，不允许被打断。
 * 集中模式结束，一定意味着水冷泵关闭。调用的时候，可以适当考虑水冷泵的关闭条件略
 * 高于target温度。
 * 
 * 增加了数组队列机制，添加了每一路的最高层五种状态定义，见APP.C/.H的修改
 * 为后期写最顶层代码做了铺垫。
 * 最顶层代码的编写，在于如下几点
 * 
 * 1 与TFT屏幕的交互，通知TFT每一路的控温状态
 * 2 控温时间的委托和处理
 * 3 综合测试
 * 见V1.1版本的后续更新
 * 
 * 
 * V 1.1版本
 * 
 * 此版本适配了最新的PCB的GPIO定义
 * 在温控改进算法方面，做了如下工作
 * 1 整理了ARG_TFT_COM相关功能，TFT的显示屏操作应该会将其任务压入队列
 * 2 整理了debug相关的功能，增加了模拟TFT屏幕，执行/停止的模拟按键指令
 * 3 还需要进一步完善对TASK处理的状态指示DEBUG功能，但是没有写完，在V1.2版本实现
 * 4 测试出了一个新的停温算法，此算法有效改进了温差的问题。后期需要把此算法插入到
 *   集中升温模式中去。 在V1.2中实现。
 *   接下来的代码编写与仿真重点在于对 队列里的 任务进行处理 显示，顺序实现每一个
 *   温度控制任务的执行管理
 * 
 * 
 * V 1.2 版本
 * 
 * 大规模完善了代码功能，具体如下
 * 
 * 1 增快了整体的系统时钟 由原先的1ms转变为100us，为了加快pwm maker的频率，降低周期
 *   使得由原先的1S周期，降低到了100ms，与分散温控算法100ms切换一次盒子相适配
 * 2 增补了PID控制器核心算法3的思想，得出了仅有大温差升温，才会进入队列顺序执行的结论
 *   并且给出了define定义来控制大温差升温，大温差降温。
 *   完成了不需要进入队列的小温差温度控制执行代码
 *   开始不需要进队列的温控算法，进入WAIT状态，然后由分散控温算法作用，直到温度达标，进入
 *   分散控温状态，结束。
 * 3 完成了road_status五种状态对TFT屏幕的通讯指示，嵌入温度读取指令中。并且完成了debug指令代码。
 * 4 确定定义了五种road_status的作用及定义
 *  TEMP_CONTORL_STOP：停止状态，此路未做任何控温/换水操作 开机默认状态
 *  TEMP_CONTROL_CHANGE_WATER：此路正在执行换水/退水操作
 *  TEMP_CONTROL_UP_DOWN_QUICK_STATUS：此路在执行集中升温控制
 *  TEMP_CONTROL_CONSTANT：此路正在分散控温，并且温度已经到达指定温度，此状态是常规状态
 *  TEMP_CONTROL_WAIT：（有队列任务时）此路正在等待其他路完成集中升温控制，本路在这一段时间得不到任何温度资源
 *                     （无队列任务时）正在控温，但是温度没有到达指定温度
 * 5 确定了 “停止操作不进队列，退水操作由开关键统一触发“ 的基本思想
 *   停止操作由两个作用 1 立刻关闭对应温控，立刻关闭水泵  2 杀死当前的与之矛盾的温控任务
 *   退水操作，当作维护功能，由电源事件统一触发
 * 
 * 还差队列任务处理函数没有完成，在写此操作之前，需要把停温算法嵌入进集中升温模式中去
 * 然后要把流程图写好，然后逐步完成队列处理函数，委托给V1.3版本。
 * 
 * 更换了KEY 与 LIQUID_FEEDBACK的引脚定义，不再使用液位传感器开关了。
 * 
 * 
 * V1.3 版本
 * 
 * 完成了所有的任务相关代码框架，待测试
 * 具体更新见PID核心算法4
 * 
 * V1.4 版本
 * 
 * 修正若干BUG，是单层的BONE版本
 * 
 * V1.5 版本
 * 
 * 增加了PID核心算法5的思想实现代码
 * 
 * 大范围降温的时候，过程如下
 * 两个温度的最大值小于目标温度时，关闭PID控制器，PWM输出为0
 * 并且保持水泵延迟停止T S 其中这个T 是由温差线性决定的，目前时用0.4作为系数
 * 例如80->40 度降温 降温16S
 * 降温的16S 分散控制器不工作。
 * 
 * 修正了 没有换水，queue_task_deal_hang_up = true;  这句不执行的BUG
 * 
 * 增加了大量的DEBUG报文
 * 
 * 减小了大范围降温的程度
 * 
 * 19.12.2 出现第二路同一个4-20mA信号不同电压值问题，改换到第三第四路 见csp_adc.c文件的修改，正常的应该改回来
 * 
 * 需要思考，如果第x路正处于集中模式，此时x路来了一个不需要排队的任务，需要如何解决问题。
 * 委托给V.16版本
 * 
 * V2.0版本
 * 
 * 修修改了GPIO定义以适应最新的电路板
 * 
 * V2.1版本
 * 
 * 优化了降温算法，降温超越时间，也就是打开水冷使得温度低于目标温度后持续的时间，被一分为二
 * 前段时间继续打开水冷 处理余温，后一段时间不打开PID控制器，使得其自己缓一会，内部处理温差
 * 然后再外力介入，控制温度。
 * 
 * V2.2版本
 * 
 * 加入了分段PID算法，见"PID控制器针对应用的核心算法7"
 * 
 * V2.3版本
 * 
 * 加入核心算法8，再分散控温的时候，对P进行增益\
 * 
 * V2.4版本
 * 加入快捷开所有风扇功能
 * 修正了分散控温模式每一路仅在奇数和偶数路选路的问题
 * 
 */

void get_version_str(uint8_t * str,uint16_t len);

#endif
