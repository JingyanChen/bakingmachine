#ifndef __CSP_GPIO__
#define __CSP_GPIO__


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
 */

//watercool system

#define WATER_COOL_PUMP_PORT_0
#define WATER_COOL_PUMP_PIN_0
#define WATER_COOL_PUMP_PORT_1
#define WATER_COOL_PUMP_PIN_1
#define WATER_COOL_PUMP_PORT_2
#define WATER_COOL_PUMP_PIN_2
#define WATER_COOL_PUMP_PORT_3
#define WATER_COOL_PUMP_PIN_3
#define WATER_COOL_PUMP_PORT_4
#define WATER_COOL_PUMP_PIN_4

#define WATER_COOL_VAVLE_PORT_0
#define WATER_COOL_VAVLE_PIN_0
#define WATER_COOL_VAVLE_PORT_1
#define WATER_COOL_VAVLE_PIN_1
#define WATER_COOL_VAVLE_PORT_2
#define WATER_COOL_VAVLE_PIN_2
#define WATER_COOL_VAVLE_PORT_3
#define WATER_COOL_VAVLE_PIN_3
#define WATER_COOL_VAVLE_PORT_4
#define WATER_COOL_VAVLE_PIN_4
#define WATER_COOL_VAVLE_PORT_5
#define WATER_COOL_VAVLE_PIN_5
#define WATER_COOL_VAVLE_PORT_6
#define WATER_COOL_VAVLE_PIN_6

#define WATER_COOL_LIQUID_FEEDBACK_PORT_0
#define WATER_COOL_LIQUID_FEEDBACK_PIN_0 
#define WATER_COOL_LIQUID_FEEDBACK_PORT_1
#define WATER_COOL_LIQUID_FEEDBACK_PIN_1
#define WATER_COOL_LIQUID_FEEDBACK_PORT_2
#define WATER_COOL_LIQUID_FEEDBACK_PIN_2
#define WATER_COOL_LIQUID_FEEDBACK_PORT_3
#define WATER_COOL_LIQUID_FEEDBACK_PIN_3
#define WATER_COOL_LIQUID_FEEDBACK_PORT_4
#define WATER_COOL_LIQUID_FEEDBACK_PIN_4



//humidity system

#define HUMIDITY_CONTROL_PORT_0
#define HUMIDITY_CONTROL_PIN_0
#define HUMIDITY_CONTROL_PORT_1
#define HUMIDITY_CONTROL_PIN_1


//motor system

#define MOTOR_LIMIT_PORT_0
#define MOTOR_LIMIT_PIN_0
#define MOTOR_LIMIT_PORT_1
#define MOTOR_LIMIT_PIN_1
#define MOTOR_LIMIT_PORT_2
#define MOTOR_LIMIT_PIN_2
#define MOTOR_LIMIT_PORT_3
#define MOTOR_LIMIT_PIN_3
#define MOTOR_LIMIT_PORT_4
#define MOTOR_LIMIT_PIN_4
#define MOTOR_LIMIT_PORT_5
#define MOTOR_LIMIT_PIN_5
#define MOTOR_LIMIT_PORT_6
#define MOTOR_LIMIT_PIN_6
#define MOTOR_LIMIT_PORT_7
#define MOTOR_LIMIT_PIN_7
#define MOTOR_LIMIT_PORT_8
#define MOTOR_LIMIT_PIN_8
#define MOTOR_LIMIT_PORT_9
#define MOTOR_LIMIT_PIN_9


//key system

#define KEY_IN_PORT_0
#define KEY_IN_PIN_0
#define KEY_IN_PORT_1
#define KEY_IN_PIN_1
#define KEY_IN_PORT_2
#define KEY_IN_PIN_2
#define KEY_IN_PORT_3
#define KEY_IN_PIN_3
#define KEY_IN_PORT_4
#define KEY_IN_PIN_4


//fan system

#define FAN_SYSTEM_CONTROL_PORT_0
#define FAN_SYSTEM_CONTROL_PIN_0
#define FAN_SYSTEM_CONTROL_PORT_1
#define FAN_SYSTEM_CONTROL_PIN_1
#define FAN_SYSTEM_CONTROL_PORT_2
#define FAN_SYSTEM_CONTROL_PIN_2
#define FAN_SYSTEM_CONTROL_PORT_3
#define FAN_SYSTEM_CONTROL_PIN_3
#define FAN_SYSTEM_CONTROL_PORT_4
#define FAN_SYSTEM_CONTROL_PIN_4

//RS485 system

#define RS485_EN_PORT
#define RS485_EN_PIN



#endif