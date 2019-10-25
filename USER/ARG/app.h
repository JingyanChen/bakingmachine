#ifndef _APP_
#define _APP_

#include "periph_motor.h"
/*
 * 
 * 本驱动提供顶级的APP软件层支持
 * 
 * 包含如下功能
 *      1 按键弹出盒子/按键收回盒子 任务
 *      2 温度控制算法
 *      
 */

#define BOX_NUM 5

typedef enum{
    box_off=0,
    box_running_forward,
    box_running_backward,
    box_on,
}box_status_t; 


#define BOX_FORWARD_DIR   CW
#define BOX_BACKWARD_DIR  CCW
#define BOX_RUNNING_TIM 5000

// 0 - 4 阻止 CW 方向
// 5 - 9 阻止 CCW 方向
void arg_app_init(void);
void arg_app_hanlde(void);


#endif