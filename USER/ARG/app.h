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
}box_status_t; 


#define BOX_FORWARD_DIR   CW
#define BOX_BACKWARD_DIR  CCW
#define BOX_RUNNING_TIM 6000

// 0 - 4 阻止 CW 方向
// 5 - 9 阻止 CCW 方向



///

#define CONTROL_TEMP_NUM 10
typedef struct{
    uint8_t control_num;
    bool control_sw[10];
    uint16_t control_temp[10];
}temp_control_t;



//为debue协议提供如下两种访问温度控制状态机的方法

//方法1 ，访问温控方法
void config_temp_control_machine(temp_control_t * temp_control);

//方法2 ，获得当前的温控状态
temp_control_t get_temp_control_machine_status(void);

void arg_app_init(void);
void arg_app_hanlde(void);


#endif
