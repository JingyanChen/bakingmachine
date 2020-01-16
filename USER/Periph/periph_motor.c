#include "periph_motor.h"
#include "csp_gpio.h"
#include "csp_pwm.h"
#include "csp_uart.h"
#include "csp_timer.h"
#include "app.h"
#include "arg_debug_pro.h"
#include <stdio.h>


static bool motor_acc_arg_sw[MOTOR_NUM];
static dir_t motor_acc_arg_running_dir[MOTOR_NUM];
static uint8_t motor_acc_arg_now_index[MOTOR_NUM];
static uint8_t motor_acc_change_speed_index[MOTOR_NUM];
static uint8_t motor_acc_arg_now_speed_index[MOTOR_NUM];
static motor_status_t motor_status[MOTOR_NUM];
static bool motor_acc_reversal[MOTOR_NUM];


//使用S曲线进行拟合 从起始速度到满速度 500 - 1000
/*
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
*/


//400-800
/*
const uint16_t motor_acc_list[ACC_LIST_LEN]={
400 ,420 ,440 ,460 ,479 ,498 ,517 ,535 ,552 ,569 ,
585 ,600 ,615 ,629 ,642 ,654 ,666 ,676 ,687 ,696 ,
705 ,713 ,720 ,727 ,733 ,739 ,745 ,750 ,754 ,758 ,
762 ,766 ,769 ,772 ,774 ,777 ,779 ,781 ,782 ,784 ,
786 ,787 ,788 ,789 ,790 ,791 ,792 ,793 ,793 ,794 , 


//上述是S曲线拟合的后半段，加速过程，减速过程是他的镜像

794 ,793 ,793 ,792 ,791 ,790 ,789 ,788 ,787 ,786 ,
784 ,782 ,781 ,779 ,777 ,774 ,772 ,769 ,766 ,762 ,
758 ,754 ,750 ,745 ,739 ,733 ,727 ,720 ,713 ,705 ,
696 ,687 ,676 ,666 ,654 ,642 ,629 ,615 ,600 ,585 ,
569 ,552 ,535 ,517 ,498 ,479 ,460 ,440 ,420 ,400 ,
};
*/


#if defined (MODERATION_MODE)
const uint16_t motor_acc_list[ACC_LIST_LEN]={


    600,620,640,660,680,700,700,700,700,700,
    700,700,700,700,700,700,700,700,700,700,
    700,700,700,700,700,700,700,700,700,700,
    700,700,700,700,700,700,700,700,700,700,
    700,700,700,700,700,700,700,700,700,700,
    700,700,700,700,700,700,700,700,700,700,
    700,700,700,700,700,700,700,700,700,700,
    700,700,700,700,700,700,700,700,700,700,
    700,700,700,700,700,700,700,700,700,700,
    700,700,700,700,700,700,680,660,640,620,

};
#endif

#if defined (MUTE_MODE)
const uint16_t motor_acc_list[ACC_LIST_LEN]={


    500,510,520,530,540,550,550,550,550,550,
    550,550,550,550,550,550,550,550,550,550,
    550,550,550,550,550,550,550,550,550,550,
    550,550,550,550,550,550,550,550,550,550,
    550,550,550,550,550,550,550,550,550,550,
    550,550,550,550,550,550,550,550,550,550,
    550,550,550,550,550,550,550,550,550,550,
    550,550,550,550,550,550,550,550,550,550,
    550,550,550,550,550,550,550,550,550,550,
    550,550,550,550,550,550,540,530,520,510,

};
#endif

#if defined (SPEED_MODE)
const uint16_t motor_acc_list[ACC_LIST_LEN]={


    999,999,999,999,999,999,999,999,999,999,
    999,999,999,999,999,999,999,999,999,999,
    999,999,999,999,999,999,999,999,999,999,
    999,999,999,999,999,999,999,999,999,999,
    999,999,999,999,999,999,999,999,999,999,
    999,999,999,999,999,999,999,999,999,999,
    999,999,999,999,999,999,999,999,999,999,
    999,999,999,999,999,999,999,999,999,999,
    999,999,999,999,999,999,999,999,999,999,
    999,999,999,999,999,999,999,999,999,999,

};
/*
const uint16_t motor_acc_list[ACC_LIST_LEN]={


    300,312,325,338,351,363,376,389,402,414,
    427,440,453,465,478,491,504,517,529,542,
    555,568,580,593,606,619,631,644,657,670,
    682,695,708,721,734,746,759,772,785,797,
    810,823,836,848,861,874,887,900,900,900,
    900,900,900,887,874,861,848,836,823,810,
    797,785,772,759,746,734,721,708,695,682,
    670,657,644,631,619,606,593,580,568,555,
    542,529,517,504,491,478,465,453,440,427,
    414,402,389,376,363,351,338,325,312,300,

};
*/

#endif
void periph_motor_init(void){
    
    uint8_t i=0;

    for(i=0;i<MOTOR_NUM;i++){
        motor_status[i] = no_running;
        clear_motor_acc_arg(i);
    }

    motor_enable_control(false);
}

static void motor_limit_handle(void){
    uint8_t i=0;

    if(_MOTOR_LIMIT_UPDATE_FLAG == false)
        return ;

    _MOTOR_LIMIT_UPDATE_FLAG = false;

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
            //注意 低位是复位限位开关，高位是终点限位开关
            if(motor_acc_arg_running_dir[i] == BOX_BACKWARD_DIR){
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

static void motor_acc_handle(void)
{

    uint8_t i = 0;
    uint8_t debug_send_buf[100];

    if (_MOTOR_ACC_CON_FLAG == false)
        return;

    _MOTOR_ACC_CON_FLAG = false;

    //10ms 速度切换handle
    for (i = 0; i < MOTOR_NUM; i++)
    {
        if (motor_acc_arg_sw[i] == true)
        {
            motor_acc_arg_now_index[i % 5]++;
            if (motor_acc_arg_now_index[i % 5] > motor_acc_change_speed_index[i % 5])
            {
                motor_acc_arg_now_index[i % 5] = 0;

                if (motor_acc_reversal[i % 5] == false)
                {
                    motor_acc_arg_now_speed_index[i % 5]++;

                    if (motor_acc_arg_now_speed_index[i % 5] > ACC_LIST_LEN - 1)
                    {
                        //运动结束
                        clear_motor_acc_arg(i);
                    }
                    else
                    {
                        set_motor_speed_dir(i % 5, (dir_t)motor_acc_arg_running_dir[i % 5], motor_acc_list[motor_acc_arg_now_speed_index[i % 5]] + S_SPEED_LIST_OFFSET);
                    }
                }
                else
                {
                    //改变运动方向，此时向后选择速度，当前的上一次速度选择是motor_acc_arg_now_speed_index[i % 5]
                    //接下来每次到更换速度时间，每次--，配置状态机的时候 保证motor_acc_arg_now_speed_index[i % 5]不是0
                    if(motor_acc_arg_now_speed_index[i % 5])
                        motor_acc_arg_now_speed_index[i % 5]--;
                    else
                        motor_acc_arg_now_speed_index[i % 5] = 0;

                    if (motor_acc_arg_now_speed_index[i % 5] == 0)
                    {
                        //运动结束
                        clear_motor_acc_arg(i);
                    }
                    else
                    {
                        set_motor_speed_dir(i % 5, (dir_t)motor_acc_arg_running_dir[i % 5], motor_acc_list[motor_acc_arg_now_speed_index[i % 5]] + S_SPEED_LIST_OFFSET);
                    }
                }
            }
            
            if(get_box_running_debug_sw()){
                if(i==0){
                    sprintf((char *)debug_send_buf,"dir : %d speed : %d",(dir_t)motor_acc_arg_running_dir[i % 5], motor_acc_list[motor_acc_arg_now_speed_index[i % 5]] + S_SPEED_LIST_OFFSET);
                    debug_sender_str(debug_send_buf);
                }    
            }
        }
    }
}

void periph_motor_handle(void){
    
    motor_limit_handle();
    motor_acc_handle();

}

void start_motor_acc_arg(uint8_t motor_id , dir_t dir ,uint16_t run_tim){

    //处于安全考虑，不接受小于1500的输入
    if(run_tim < 1000)
        return ;

    motor_acc_arg_sw[motor_id % 5] = true;
    motor_acc_arg_now_index[motor_id % 5] = 0;
    motor_acc_arg_now_speed_index[motor_id % 5] = 0;
    motor_acc_change_speed_index[motor_id % 5] = run_tim / 100;
    motor_acc_arg_running_dir[motor_id % 5] = dir;

    if(motor_acc_reversal[motor_id % 5] == false)
        motor_acc_reversal[motor_id % 5] = true;
    else
        motor_acc_reversal[motor_id % 5] = false;


    motor_status[motor_id % 5] = is_running;

    set_motor_speed_dir(motor_id % 5 ,dir , motor_acc_list[motor_acc_arg_now_speed_index[motor_id % 5]]);
}


bool start_motor_acc_arg_return(uint8_t motor_id){

    if(motor_acc_arg_sw[motor_id % 5] == false)
        return false;
    
    if(motor_status[motor_id % 5] != is_running)
        return false;

    if(motor_acc_arg_now_speed_index[motor_id % 5] == 0)
        return false;

    if(motor_acc_reversal[motor_id % 5] == false)
        motor_acc_reversal[motor_id % 5] = true;
    else
        motor_acc_reversal[motor_id % 5] = false;

    //改变运动方向

    if(motor_acc_arg_running_dir[motor_id % 5] == CCW){
        motor_acc_arg_running_dir[motor_id % 5] = CW;
    }else if(motor_acc_arg_running_dir[motor_id % 5] == CW){
        motor_acc_arg_running_dir[motor_id % 5] = CCW;
    } 

    //给初始速度

     set_motor_speed_dir(motor_id % 5 ,motor_acc_arg_running_dir[motor_id % 5] , motor_acc_list[motor_acc_arg_now_speed_index[motor_id % 5]]);
    return true;
    
}

void clear_motor_acc_arg(uint8_t motor_id){

    motor_acc_arg_sw[motor_id % 5] = false;
    motor_acc_arg_now_index[motor_id % 5] = 0;
    motor_acc_arg_now_speed_index[motor_id % 5] = 0;
    motor_acc_change_speed_index[motor_id % 5] = 0;
    motor_acc_reversal[motor_id % 5] = true;

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
        motor_dir_set(id % 5 , false);
    }else{
        motor_dir_set(id % 5 , true);
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
