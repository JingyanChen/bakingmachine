#include <stdbool.h>
#include "csp_pwm.h"
#include "csp_timer.h"
#include "csp_gpio.h"
#include "sys.h"



GPIO_TypeDef *PWM_PORT_LIST[]={
        PWM_CONTROL_PORT_0,
        PWM_CONTROL_PORT_1,
        PWM_CONTROL_PORT_2,
        PWM_CONTROL_PORT_3,
        PWM_CONTROL_PORT_4,
        PWM_CONTROL_PORT_5,
        PWM_CONTROL_PORT_6,
        PWM_CONTROL_PORT_7,
        PWM_CONTROL_PORT_8,
        PWM_CONTROL_PORT_9,
};

uint16_t PWM_PIN_LIST[]={
        PWM_CONTROL_PIN_0 ,
        PWM_CONTROL_PIN_1 ,  
        PWM_CONTROL_PIN_2 ,  
        PWM_CONTROL_PIN_3 , 
        PWM_CONTROL_PIN_4 ,    
        PWM_CONTROL_PIN_5 ,  
        PWM_CONTROL_PIN_6 ,     
        PWM_CONTROL_PIN_7 ,
        PWM_CONTROL_PIN_8 ,
        PWM_CONTROL_PIN_9 ,
};


#define TIMER_PERIOD 4449
//#define TIMER_PERIOD 65534
void csp_pwm_init(void)
{
		uint8_t i=0;
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    //使用TIM3 CH2 提供一路 PB5 PWM输出
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);

    GPIO_InitStructure.GPIO_Pin = MOTOR_CONTROL_PIN_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_CONTROL_PORT_0, &GPIO_InitStructure);

    TIM_TimeBaseStructure.TIM_Period = TIMER_PERIOD; //frequency  = 8KHZ
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC2Init(TIM3, &TIM_OCInitStructure);

    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);

    TIM_Cmd(TIM3, ENABLE);

    //使用TIM2 CH2 提供一路 PA1 PWM输出
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = MOTOR_CONTROL_PIN_1; // TIM2_CH2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_CONTROL_PORT_1, &GPIO_InitStructure);

    TIM_TimeBaseStructure.TIM_Period = TIMER_PERIOD;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);

    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable); //??TIM2?CCR2????????

    TIM_Cmd(TIM2, ENABLE);

    //使用TIM4 CH2 3 4 提供三路PWM输出

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //

    GPIO_InitStructure.GPIO_Pin = MOTOR_CONTROL_PIN_2; //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(MOTOR_CONTROL_PORT_2, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = MOTOR_CONTROL_PIN_3; //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(MOTOR_CONTROL_PORT_3, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = MOTOR_CONTROL_PIN_4; //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(MOTOR_CONTROL_PORT_4, &GPIO_InitStructure);

    TIM_TimeBaseStructure.TIM_Period = TIMER_PERIOD; //
    TIM_TimeBaseStructure.TIM_Prescaler = 0; //
    TIM_TimeBaseStructure.TIM_ClockDivision = 0; //
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //

    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //
    TIM_OCInitStructure.TIM_Pulse = 0; //
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //

    TIM_OC1Init(TIM4, &TIM_OCInitStructure); //

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //
    TIM_OCInitStructure.TIM_Pulse = 0; //
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //

    TIM_OC2Init(TIM4, &TIM_OCInitStructure); //

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //
    TIM_OCInitStructure.TIM_Pulse = 0; //
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //

    TIM_OC4Init(TIM4, &TIM_OCInitStructure); //

    TIM_CtrlPWMOutputs(TIM4, ENABLE); //

    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable); //
    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable); //
    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable); //
    TIM_ARRPreloadConfig(TIM4, ENABLE); //
    TIM_Cmd(TIM4, ENABLE); //

    for (i = 0; i < 5; i++)
    {
        set_pwm(0,0);
    }

    //软件PWM GPIO CONFIG

    for (i = 0; i < 10; i++)
    {

        GPIO_InitStructure.GPIO_Pin = PWM_PIN_LIST[i];
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(PWM_PORT_LIST[i], &GPIO_InitStructure);
        GPIO_ResetBits(PWM_PORT_LIST[i], PWM_PIN_LIST[i]);
    }    

}
/*
 * percent 是0-1000的数字，如果500，则意味着一个周期内500ms高电平500ms低电平
 * 周期默认是1000ms也就是1S的时间，十路同时计数，计数到对应percent，变为低电平
 * 如果percent为0，则不打开pluse maker状态机
 */
static uint16_t pwm_maker_percent[]={0,0,0,0,0,0,0,0,0,0};
static void set_pwm_maker_percent(uint8_t id , uint16_t percent){
    pwm_maker_percent[id % 10] = percent;
}
uint16_t get_pwm_maker_percent(uint8_t id){
    return pwm_maker_percent[id % 10];
}


static void access_pwm_gpio_v(uint8_t id , bool sw){
    if(sw){
        GPIO_SetBits(PWM_PORT_LIST[id % 10] , PWM_PIN_LIST[id % 10]);
    }else{
        GPIO_ResetBits(PWM_PORT_LIST[id % 10 ] , PWM_PIN_LIST[id % 10]);
    }
} 

//per 1ms make once pluses
static uint16_t pwm_maker_period_tick_ms=0;
/*
 * 2019.11.25 加入保护代码，同一时刻仅允许两个GPIO置1，否则会出现功率过大的问题
 * 
 * 11.25之前的版本，使用的PWM波形周期是1S ，但是分散控温模式，每一次切换，仅给了
 * 100ms的时间，所以PID控制器只能显示1/10的效能，感觉不合理，所以在11.25版本之后
 * PWM的周期改为100ms，使得每一次切换的时候PID控制器都可以发挥最大效能。
 * 
 * 系统的SYS_TICK由1ms 提速到100us
 * 更改见于 csp_timer.c/.h
 */
void csp_pwm_handle(void)
{
    uint8_t i=0;
    uint8_t gpio_high_tick=0;
     
    if(_PLUSE_MAKER_FLAG == false)
        return ;
    
    _PLUSE_MAKER_FLAG = false;
    
    for(i=0;i<10;i++){
        if(pwm_maker_percent[i] != 0){

            gpio_high_tick++;
            if(gpio_high_tick > 2)
                break;

            if(pwm_maker_period_tick_ms < get_pwm_maker_percent(i))
                access_pwm_gpio_v(i,true);
            else
                access_pwm_gpio_v(i,false);
        }else{
				access_pwm_gpio_v(i,false);
		}
    }

    pwm_maker_period_tick_ms++;

    if(pwm_maker_period_tick_ms > pwm_maker_period_ms){
        pwm_maker_period_tick_ms = 0;
    }

}


void set_pwm(uint8_t pwm_id , float percent){
    uint16_t pwm = 0;

    pwm = (uint16_t)((float)TIMER_PERIOD * percent);

    switch(pwm_id){
        case 0:TIM_SetCompare2(TIM2,pwm);break;        
        case 1:TIM_SetCompare2(TIM3,pwm);break;
        case 2:TIM_SetCompare1(TIM4,pwm);break;
        case 3:TIM_SetCompare2(TIM4,pwm);break;
        case 4:TIM_SetCompare4(TIM4,pwm);break;
        default : break;
    }
}

//待示波器测试，不输出脉冲的时候最好保持低电平
void close_pwm(uint8_t pwm_id){
    switch(pwm_id){
        case 0:TIM_SetCompare2(TIM2,0);break;        
        case 1:TIM_SetCompare2(TIM3,0);break;
        case 2:TIM_SetCompare1(TIM4,0);break;
        case 3:TIM_SetCompare2(TIM4,0);break;
        case 4:TIM_SetCompare4(TIM4,0);break;
        default : break;
    }    
}

bool set_software_pwm(uint8_t pwm_id , uint16_t percent){
    set_pwm_maker_percent(pwm_id,percent % 1001);
    return true;
}

void close_software_pwm(uint8_t pwm_id){
    set_pwm_maker_percent(pwm_id,0);
}

void close_all_software_pwm_out_except(uint8_t id1,uint8_t id2){
    uint8_t i=0;
    for(i=0;i<10;i++){
        if(i!=id1 && i!=id2){
            set_pwm_maker_percent(i,0);
            access_pwm_gpio_v(i,false);
        }
    }
}

void close_all_software_pwm_out(void){
    uint8_t i=0;
    for(i=0;i<10;i++){
            set_pwm_maker_percent(i,0);
            access_pwm_gpio_v(i,false);
    }
}
