#ifndef __ARG_PID__
#define __ARG_PID__

#include <stdint.h>
#include <stdbool.h>


/*
 * 本驱动旨在提供一个PD控制器驱动，在handle中，PD控制器会一直获取不断被更新的温度值
 * 求出error之后，会将error通过PD算法换算为0-1000的PWM占空比数值，通过加热片反馈给温度
 * pid算法仅在升温的时候有效，降温的时候做一些动作来快速降温
 */


//打开此宏定义，PID的计算过程将会每一步都展现在DEBUG串口中
#define PID_DEBUG_LOG_SW 
#define PID_CONTORLLER_NUM 10


#define P 0.002
#define D 0.001

void arg_pid_init(void);
void arg_pid_handle(void);



#endif
