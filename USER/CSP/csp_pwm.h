#ifndef __PWM__
#define __PWM__

#include <stdint.h>
#include <stdbool.h>

/*
 * 本驱动旨在编写完成 
 *  1 10路PWM占空比可调程序，使用pwm_maker状态机实现
 *  2 5路固定频率，占空比可调，有使能开关的PWM信号，使用硬件的PWM模块实现
 * 
 *  考虑到本项目不会出现步进电机频率可变的情况，所以所有PWM的频率都是由宏定义来确定的
 *  由烧写程序来确定，通过修改其占空比从而来控制其运动速度
 */


#define PWM_CONTROL_PORT_0  GPIOF
#define PWM_CONTROL_PIN_0   GPIO_Pin_0
#define PWM_CONTROL_PORT_1  GPIOF
#define PWM_CONTROL_PIN_1   GPIO_Pin_1
#define PWM_CONTROL_PORT_2  GPIOF
#define PWM_CONTROL_PIN_2   GPIO_Pin_2
#define PWM_CONTROL_PORT_3  GPIOF
#define PWM_CONTROL_PIN_3   GPIO_Pin_3
#define PWM_CONTROL_PORT_4  GPIOF
#define PWM_CONTROL_PIN_4   GPIO_Pin_4
#define PWM_CONTROL_PORT_5  GPIOF
#define PWM_CONTROL_PIN_5   GPIO_Pin_5
#define PWM_CONTROL_PORT_6  GPIOF
#define PWM_CONTROL_PIN_6   GPIO_Pin_6
#define PWM_CONTROL_PORT_7  GPIOF
#define PWM_CONTROL_PIN_7   GPIO_Pin_7
#define PWM_CONTROL_PORT_8  GPIOF
#define PWM_CONTROL_PIN_8   GPIO_Pin_8
#define PWM_CONTROL_PORT_9  GPIOF
#define PWM_CONTROL_PIN_9   GPIO_Pin_9


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


//理论频率计算方法
//72000000 / TIMER_PERIOD + 1 = 8000hz 需要修改频率参考此公式

/*
 * brief : set pwm_id as percent duty cycle
 * 
 * @ pra pwm_id : 0-4 pwm id 
 * @ percent : duty cycle 
 * 
 * example :
 *          set_pwm(0,0.3) pwm 0 will output 30% pwm
 */
void set_pwm(uint8_t pwm_id , float percent);

/*
 * brief : close pwm output 
 * 
 * @ pra pwm_id : 0-4 pwm id 
 * 
 * example :
 *          set_pwm(0)
 */
void close_pwm(uint8_t pwm_id);

//TODO 10.16
//需要进一步通过定时器观察确定代码无误，五路可控占空比PWM是独立的，且可以快速更改占空比


//pluse maker 状态机实现方法
/*
 * pluser maker ,采用1ms的定时信号，模拟500HZ的可控占空比信号
 * 每1ms判断一次是否需要转换电平，1S为一个周期，也就是pluse maker由1000的细分度
 * 对外API
 * 
 * set_software_pwm(pwm_id , uint16_t percent);
 * 
 * 内部的运行机理
 * 
 * csp_pwm_handle()内部1ms对percent进行一次快速解析，给出对应电平
 * 
 * 由于是软件模拟的PWM，所以当有其他事务处理的时候，会微小的影响占空比，但是影响不大。
 * 主线程不允许被卡死。更不允许出现delay空转的情况。
 * 
 */

#define  pwm_maker_period_ms 1000

/*
 * brief : set pluse maker duty cycle
 * pra @ pwm_id  : pwd id 
 * pra @ perenct : 0-1000 , --- 0 -100.0% duty cycles
 * return : set is success?
 */
bool set_software_pwm(uint8_t pwm_id , uint16_t percent);

/*
 * brief : close pluse maker 
 * pra @ pwm_id  : pwd id 
 */
void close_software_pwm(uint8_t pwm_id);
#endif

