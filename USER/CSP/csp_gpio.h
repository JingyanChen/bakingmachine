#ifndef __CSP_GPIO__
#define __CSP_GPIO__

#include  <stdbool.h>

/*
 * GPIO define for barkingMachine
 * GPIO DEVICE is follow
 * 
 * watercool system
 * 
 * IN 5 OUT 12
 * 
 * 五个水冷泵的输出控制
 * 七个水流入流出的电磁阀控制
 * 五个液位电平感应传感器
 * 
 * humidity system
 * 
 * OUT 2
 * 
 * 两路加湿度的设备
 * 
 * motor system
 * 
 * IN 10
 * 
 * 五路限位开关 + 5路备用开关
 * 
 * key system
 * 
 * IN 5
 * 
 * 五路电容按键输入
 * 一路开关机输入
 *
 * 
 * fan system
 * 
 * OUT 5
 * 
 * 五路维护风扇
 * 
 * 
 * RS485抄读 PT100温度网络数据 使能端
 * 
 * OUT 1
 * 
 * LED SYSTEM
 * 
 * OUT1
 * 
 */

//watercool system

#define WATER_COOL_PUMP_PORT_0  GPIOE
#define WATER_COOL_PUMP_PIN_0   GPIO_Pin_0
#define WATER_COOL_PUMP_PORT_1  GPIOE
#define WATER_COOL_PUMP_PIN_1   GPIO_Pin_1
#define WATER_COOL_PUMP_PORT_2  GPIOE
#define WATER_COOL_PUMP_PIN_2   GPIO_Pin_2    
#define WATER_COOL_PUMP_PORT_3  GPIOE
#define WATER_COOL_PUMP_PIN_3   GPIO_Pin_3
#define WATER_COOL_PUMP_PORT_4  GPIOE
#define WATER_COOL_PUMP_PIN_4   GPIO_Pin_4

#define WATER_COOL_VAVLE_PORT_0 GPIOE
#define WATER_COOL_VAVLE_PIN_0  GPIO_Pin_5
#define WATER_COOL_VAVLE_PORT_1 GPIOE
#define WATER_COOL_VAVLE_PIN_1  GPIO_Pin_6
#define WATER_COOL_VAVLE_PORT_2 GPIOE
#define WATER_COOL_VAVLE_PIN_2  GPIO_Pin_7
#define WATER_COOL_VAVLE_PORT_3 GPIOE
#define WATER_COOL_VAVLE_PIN_3  GPIO_Pin_8
#define WATER_COOL_VAVLE_PORT_4 GPIOE
#define WATER_COOL_VAVLE_PIN_4  GPIO_Pin_9
#define WATER_COOL_VAVLE_PORT_5 GPIOE
#define WATER_COOL_VAVLE_PIN_5  GPIO_Pin_10
#define WATER_COOL_VAVLE_PORT_6 GPIOE
#define WATER_COOL_VAVLE_PIN_6  GPIO_Pin_11

#define WATER_COOL_LIQUID_FEEDBACK_PORT_0   GPIOE
#define WATER_COOL_LIQUID_FEEDBACK_PIN_0    GPIO_Pin_12
#define WATER_COOL_LIQUID_FEEDBACK_PORT_1   GPIOE
#define WATER_COOL_LIQUID_FEEDBACK_PIN_1    GPIO_Pin_13
#define WATER_COOL_LIQUID_FEEDBACK_PORT_2   GPIOE
#define WATER_COOL_LIQUID_FEEDBACK_PIN_2    GPIO_Pin_14
#define WATER_COOL_LIQUID_FEEDBACK_PORT_3   GPIOE
#define WATER_COOL_LIQUID_FEEDBACK_PIN_3    GPIO_Pin_15
#define WATER_COOL_LIQUID_FEEDBACK_PORT_4   GPIOF
#define WATER_COOL_LIQUID_FEEDBACK_PIN_4    GPIO_Pin_1



//humidity system

#define HUMIDITY_CONTROL_PORT_0 GPIOF
#define HUMIDITY_CONTROL_PIN_0  GPIO_Pin_2
#define HUMIDITY_CONTROL_PORT_1 GPIOF
#define HUMIDITY_CONTROL_PIN_1  GPIO_Pin_3


//motor system

#define MOTOR_LIMIT_PORT_0  GPIOF
#define MOTOR_LIMIT_PIN_0   GPIO_Pin_4
#define MOTOR_LIMIT_PORT_1  GPIOF
#define MOTOR_LIMIT_PIN_1   GPIO_Pin_5
#define MOTOR_LIMIT_PORT_2  GPIOF
#define MOTOR_LIMIT_PIN_2   GPIO_Pin_6
#define MOTOR_LIMIT_PORT_3  GPIOF
#define MOTOR_LIMIT_PIN_3   GPIO_Pin_7
#define MOTOR_LIMIT_PORT_4  GPIOF
#define MOTOR_LIMIT_PIN_4   GPIO_Pin_8
#define MOTOR_LIMIT_PORT_5  GPIOF
#define MOTOR_LIMIT_PIN_5   GPIO_Pin_9
#define MOTOR_LIMIT_PORT_6  GPIOF
#define MOTOR_LIMIT_PIN_6   GPIO_Pin_10
#define MOTOR_LIMIT_PORT_7  GPIOF
#define MOTOR_LIMIT_PIN_7   GPIO_Pin_11
#define MOTOR_LIMIT_PORT_8  GPIOF
#define MOTOR_LIMIT_PIN_8   GPIO_Pin_12
#define MOTOR_LIMIT_PORT_9  GPIOF
#define MOTOR_LIMIT_PIN_9   GPIO_Pin_13


#define MOTOR_DIR_PORT_0    GPIOF
#define MOTOR_DIR_PIN_0     GPIO_Pin_14
#define MOTOR_DIR_PORT_1    GPIOF
#define MOTOR_DIR_PIN_1     GPIO_Pin_15
#define MOTOR_DIR_PORT_2    GPIOD
#define MOTOR_DIR_PIN_2     GPIO_Pin_10
#define MOTOR_DIR_PORT_3    GPIOD
#define MOTOR_DIR_PIN_3     GPIO_Pin_11
#define MOTOR_DIR_PORT_4    GPIOD
#define MOTOR_DIR_PIN_4     GPIO_Pin_12

//key system

#define KEY_IN_PORT_0   GPIOD
#define KEY_IN_PIN_0    GPIO_Pin_13
#define KEY_IN_PORT_1   GPIOD
#define KEY_IN_PIN_1    GPIO_Pin_14
#define KEY_IN_PORT_2   GPIOD
#define KEY_IN_PIN_2    GPIO_Pin_15
#define KEY_IN_PORT_3   GPIOC
#define KEY_IN_PIN_3    GPIO_Pin_15
#define KEY_IN_PORT_4   GPIOC
#define KEY_IN_PIN_4    GPIO_Pin_14
#define KEY_POWER_PORT  GPIOC
#define KEY_POWER_PIN   GPIO_Pin_13


//fan system

#define FAN_SYSTEM_CONTROL_PORT_0   GPIOC
#define FAN_SYSTEM_CONTROL_PIN_0    GPIO_Pin_12
#define FAN_SYSTEM_CONTROL_PORT_1   GPIOC
#define FAN_SYSTEM_CONTROL_PIN_1    GPIO_Pin_11
#define FAN_SYSTEM_CONTROL_PORT_2   GPIOC
#define FAN_SYSTEM_CONTROL_PIN_2    GPIO_Pin_9
#define FAN_SYSTEM_CONTROL_PORT_3   GPIOC
#define FAN_SYSTEM_CONTROL_PIN_3    GPIO_Pin_8
#define FAN_SYSTEM_CONTROL_PORT_4   GPIOC
#define FAN_SYSTEM_CONTROL_PIN_4    GPIO_Pin_7

//RS485 system

#define RS485_EN_PORT   GPIOC
#define RS485_EN_PIN    GPIO_Pin_6

//LED 

#define LED_PORT   GPIOC
#define LED_PIN    GPIO_Pin_5



void csp_gpio_init(void);
void csp_gpio_handle(void);



// output device API

/*
 * brief set water cool pump control
 */
void water_cool_pump_control(uint8_t pump_id , bool sw);
/*
 * brief set water cool vavle control
 */
void water_cool_vavle_control(uint8_t vavle_id , bool sw);
/*
 * brief set humidity control
 */
void humidity_control(uint8_t humidity_id , bool sw);
/*
 * brief set fan contorl
 */
void fan_control(uint8_t id , bool sw);
/*
 * brief rs485 enable contorl
 */
void rs485_enbale_control(bool sw);
/*
 * brief power led control
 */
void power_led_control(bool sw);

//input device API
/*
 * brief get liquid feedback volatle
 */
bool get_liquid_feedback_v(uint8_t id);

/*
 * brief get motor limit 
 */
bool get_motor_limit_v(uint8_t id);
/*
 * get key in v
 */
bool get_key_in_v(uint8_t id);
/*
 * get power key in
 */
bool get_power_key_in(void);
/*
 * set motor dir
 */
void motor_dir_set(uint8_t id , bool sw);
#endif
