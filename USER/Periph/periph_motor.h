#ifndef __motor__
#define __motor__


#include <stdint.h>


#define MOTOR_NUM 5
#define ACC_LIST_LEN 100

/*
 * 当板子没有接任何按键的时候，默认得到数据是1
 * 设备必须所有限位按键都达到下面这个宏定义的值才能正常启动，认为所有盒子都回归原位了
 * 所以如果想不挂载按键的情况下就可以测试系统性能，把下述改为1
 * 正常使用的时候，改回0
 * 刚开始，盒子状态一定是 unknown状态，只有所有盒子正确复位之后才能回归到off状态
 */
#define MOTOR_LIMIT_V 0   //当限位开关电平展示为MOTOR_LIMIT_V时，无条件阻止电机运动
/*
 * 本驱动旨在在CSP_PWM的基础上，配合时间片轮询的工程结构，完成一个电机动态变速的功能
 * 考虑100ms变化一次pwm的占空比，1S变化10次，通过编程输入占空比曲线，从而实现电机的加减速算法
 * 通过时间来控制脉冲数，也就是说，有两种方式改变位移 1 是改变时间的长短 2 是改变加减速的曲线
 * 
 * 本驱动会增添无条件的光电开关限位代码，光电开关限位是无条件的阻止电机运动。
 * 
 * 本驱动可以在DEBUG指令模式下进行调试，DEBUG抽象为执行开关机动作指令
 * 
 * 
 * 驱动构建方法如下，初始状态保证五路PWM 均输出占空比为0的方波
 * 当出现开始运动的条件之后，状态机在总时间T的 T/200 S 变化一次速度，需要给出200个占空比值，暂定200，依据工程
 * 需要在做轻微修改
 * 
 * 结束条件为，触碰光电开关
 */

typedef enum{
    CW=0,
    CCW,
}dir_t;

void periph_motor_init(void);
void periph_motor_handle(void);

/*
 * brief 控制电机的方向和转速
 * pra id 0-4 五路电机
 * pra speed 0-1000的数字，越大速度越快，不可超过1000
 * pra dir 方向 CW/CCW
 */
void set_motor_speed_dir(uint8_t id , dir_t dir ,uint16_t speed);

/*
 * brief 无条件关闭motor
 * pra id 0-4 五路电机的id
 */
void close_motor(uint8_t id);


typedef enum{
    no_running=0,
    stop_by_sw,
    stop_by_run_done,
    stop_by_commander,
    is_running,
}motor_status_t;

/*
 * brief 开始直流无刷电机的加减速算法动作
 *       一旦打开了此状态机，如果中途停止，需要清空状态机变量
 *      此状态机，停止条件有二 ，触碰光电开关，或者运动结束
 *      打开之后，会依照 总时间/100 更改一次速度的原则，改变100次速度
 *      100ms作为一次计时，以S级为基本单位
 *      例如 3000ms 运动行程 则30ms改变一次运动速度，改变100次，运动结束
 *      在此期间，如果出现光电开关被按下的情况，一定迅速停止状态机
 *      运动结束的正常流程，一定是触碰光电开关，如果不触碰，则一直运动到触碰为止
 * 
 * pra motor_id : 电机的id 0-4
 * pra run_tim : 运行的时间 单位ms
 */
void start_motor_acc_arg(uint8_t motor_id , dir_t dir  , uint16_t run_tim);

//清空加减速状态机
void clear_motor_acc_arg(uint8_t motor_id);

//获得电机当前的运动状态
motor_status_t get_motor_status(uint8_t motor_id);
#endif

