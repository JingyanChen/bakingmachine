#ifndef __PWM__
#define __PWM__

#include <stdint.h>


/*
 * 本驱动旨在编写完成 
 *  1 10路PWM占空比可调程序，使用pwm_maker状态机实现
 *  2 5路固定频率，占空比可调，有使能开关的PWM信号，使用硬件的PWM模块实现
 * 
 *  考虑到本项目不会出现步进电机频率可变的情况，所以所有PWM的频率都是由宏定义来确定的
 *  由烧写程序来确定，通过修改其占空比从而来控制其运动速度
 */


#define PWM_CONTROL_PORT_0
#define PWM_CONTROL_PIN_0
#define PWM_CONTROL_PORT_1
#define PWM_CONTROL_PIN_1
#define PWM_CONTROL_PORT_2
#define PWM_CONTROL_PIN_2
#define PWM_CONTROL_PORT_3
#define PWM_CONTROL_PIN_3
#define PWM_CONTROL_PORT_4
#define PWM_CONTROL_PIN_4
#define PWM_CONTROL_PORT_5
#define PWM_CONTROL_PIN_5
#define PWM_CONTROL_PORT_6
#define PWM_CONTROL_PIN_6
#define PWM_CONTROL_PORT_7
#define PWM_CONTROL_PIN_7
#define PWM_CONTROL_PORT_8
#define PWM_CONTROL_PIN_8
#define PWM_CONTROL_PORT_9
#define PWM_CONTROL_PIN_9


//下列的引脚定义不允许修改，因为和底层硬件定义紧密相连
#define MOTOR_CONTROL_PORT_0    GPIOB
#define MOTOR_CONTROL_PIN_0     GPIO_Pin_5
#define MOTOR_CONTROL_PORT_1    GPIOA 
#define MOTOR_CONTROL_PIN_1     GPIO_Pin_1
#define MOTOR_CONTROL_PORT_2    GPIOB
#define MOTOR_CONTROL_PIN_2     GPIO_Pin_6
#define MOTOR_CONTROL_PORT_3    GPIOB
#define MOTOR_CONTROL_PIN_3     GPIO_Pin_7
#define MOTOR_CONTROL_PORT_4    GPIOB
#define MOTOR_CONTROL_PIN_4     GPIO_Pin_9

void csp_pwm_init(void);
void csp_pwm_handle(void);
void set_pwm(uint8_t pwm_id , float percent);

#endif

