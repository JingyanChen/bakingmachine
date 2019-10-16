#include "csp_pwm.h"
#include "sys.h"

#define TIMER_PERIOD 8999
void csp_pwm_init(void)
{
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

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
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

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
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
}


void csp_pwm_handle(void)
{
}


void set_pwm(uint8_t pwm_id , float percent){
    uint16_t pwm = 0;

    pwm = (uint16_t)((float)TIMER_PERIOD * percent);

    switch(pwm_id){
        case 0:TIM_SetCompare2(TIM3,pwm);break;
        case 1:TIM_SetCompare2(TIM2,pwm);break;
        case 2:TIM_SetCompare1(TIM4,pwm);break;
        case 3:TIM_SetCompare2(TIM4,pwm);break;
        case 4:TIM_SetCompare4(TIM4,pwm);break;
        default : break;
    }
}
