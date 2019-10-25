#include "periph_motor.h"
#include "csp_gpio.h"
#include "csp_pwm.h"
#include "csp_uart.h"
#include "csp_timer.h"

static bool motor_acc_arg_sw[MOTOR_NUM];
static dir_t motor_acc_arg_running_dir[MOTOR_NUM];
static uint8_t motor_acc_arg_now_index[MOTOR_NUM];
static uint8_t motor_acc_change_speed_index[MOTOR_NUM];
static uint8_t motor_acc_arg_now_speed_index[MOTOR_NUM];
static motor_status_t motor_status[MOTOR_NUM];

//使用S曲线进行拟合 从起始速度到满速度
const uint16_t motor_acc_list[ACC_LIST_LEN]={
500 ,525 ,550 ,574 ,599 ,622 ,646 ,668 ,690 ,711 ,
731 ,750 ,769 ,786 ,802 ,818 ,832 ,846 ,858 ,870 ,
881 ,891 ,900 ,909 ,917 ,924 ,931 ,937 ,943 ,948 ,
953 ,957 ,961 ,964 ,968 ,971 ,973 ,976 ,978 ,980 ,
982 ,984 ,985 ,987 ,988 ,989 ,990 ,991 ,992 ,993 ,

//上述是S曲线拟合的后半段，加速过程，减速过程是他的镜像

993 ,992 ,991 ,990 ,989 ,988 ,987 ,985 ,984 ,982 ,
980 ,978 ,976 ,973 ,971 ,968 ,964 ,961 ,957 ,953 ,
948 ,943 ,937 ,931 ,924 ,917 ,909 ,900 ,891 ,881 ,
870 ,858 ,846 ,832 ,818 ,802 ,786 ,769 ,750 ,731 ,
711 ,690 ,668 ,646 ,622 ,599 ,574 ,550 ,525 ,500 ,
};

void periph_motor_init(void){
    uint8_t i=0;
    for(i=0;i<MOTOR_NUM;i++){
        motor_status[i] = no_running;
        clear_motor_acc_arg(i);
    }
}

//10ms运行一次
void periph_motor_handle(void){
    uint8_t i=0;

    if(_MOTOR_ACC_CON_FLAG == false)
        return ;

    _MOTOR_ACC_CON_FLAG = false;

    //10ms 速度切换handle
    for(i=0;i<MOTOR_NUM;i++){
        if(motor_acc_arg_sw[i] == true){
            motor_acc_arg_now_index[i % 5] ++;
            if(motor_acc_arg_now_index[i % 5] > motor_acc_change_speed_index[i % 5]){
                motor_acc_arg_now_index[i % 5]  = 0;

                motor_acc_arg_now_speed_index[i % 5]++;

                if(motor_acc_arg_now_speed_index[i % 5] > ACC_LIST_LEN - 1){
                    //运动结束
                    clear_motor_acc_arg(i);
                }else{
                    set_motor_speed_dir(i % 5 ,(dir_t)motor_acc_arg_running_dir[i % 5] , motor_acc_list[motor_acc_arg_now_speed_index[i % 5]]);             
                }   
            }
        }
    }

    //10ms 光电开关检测handle
    //默认配置如下
    /*
     * (0,5) 阻止 0号电机的 (CW,CCW)
     * (1,6) 阻止 1号电机的 (CW,CCW)
     * (2,7) 阻止 2号电机的 (CW,CCW)
     * (3,8) 阻止 3号电机的 (CW,CCW)
     * (4,9) 阻止 4号电机的 (CW,CCW)
     */
    for(i=0;i<MOTOR_NUM;i++){
        if(motor_status[i] == is_running){
            //发现电机在运动，判断其运动方向

            if(motor_acc_arg_running_dir[i] == CW){
                if(get_motor_limit_v(i) == MOTOR_LIMIT_V){
                    close_motor(i);
                    clear_motor_acc_arg(i);
                }
            }else{
                if(get_motor_limit_v(5+i) == MOTOR_LIMIT_V){
                    close_motor(i);
                    clear_motor_acc_arg(i);
                }
            }
        }
    }    

}

void start_motor_acc_arg(uint8_t motor_id , dir_t dir ,uint16_t run_tim){

    //处于安全考虑，不接受小于2000的输入
    if(run_tim < 2000)
        return ;

    motor_acc_arg_sw[motor_id % 5] = true;
    motor_acc_arg_now_index[motor_id % 5] = 0;
    motor_acc_arg_now_speed_index[motor_id % 5] = 0;
    motor_acc_change_speed_index[motor_id % 5] = run_tim / 1000;
    motor_acc_arg_running_dir[motor_id % 5] = dir;

    motor_status[motor_id % 5] = is_running;

    set_motor_speed_dir(motor_id % 5 ,dir , motor_acc_list[motor_acc_arg_now_speed_index[motor_id % 5]]);
}

void clear_motor_acc_arg(uint8_t motor_id){

    motor_acc_arg_sw[motor_id % 5] = false;
    motor_acc_arg_now_index[motor_id % 5] = 0;
    motor_acc_arg_now_speed_index[motor_id % 5] = 0;
    motor_acc_change_speed_index[motor_id % 5] = 0;


    //目前层面的封装，让运动结束之后，不停止电机，以最低速度运行，一直等到光电开关作用为止
    //motor_acc_arg_running_dir[motor_id % 5] = CW;    
    //close_motor(motor_id);
}

void set_motor_speed_dir(uint8_t id , dir_t dir ,uint16_t speed){
    float speed_float = 0.0;

    if(speed > 1000)
        return ;
    
    speed_float = (float)speed / 1000.0;

    if(dir == CW){
        motor_dir_set(id % 5 , true);
    }else{
        motor_dir_set(id % 5 , false);
    }

    set_pwm(id % 5,speed_float);
    
    if(speed != 0)
        motor_status[id % 5] = is_running;  

    motor_acc_arg_running_dir[id % 5] = dir;

}

void close_motor(uint8_t id){
    set_pwm(id % 5 , 0);
    motor_status[id % 5] = no_running;
}


motor_status_t get_motor_status(uint8_t motor_id){
    return motor_status[motor_id % 5];
}
