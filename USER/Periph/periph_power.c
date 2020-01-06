#include <stdbool.h>
#include "periph_power.h"
#include "csp_gpio.h"
#include "csp_uart.h"
#include "periph_key.h"
#include "arg_debug_pro.h"
#include "sys.h"
#include "delay.h"
#include "periph_fan.h"
#include "arg_pid.h"
#include "periph_motor.h"
#include "periph_key.h"
#include "periph_humidity_sys.h"
#include "csp_wtd.h"
#include "app.h"
#include "csp_pwm.h"
#include "csp_timer.h"
static lcd_power_status_t lcd_power_status;

//待机相关代码

static void config_sys_key_as_interrupt(void)
{

    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource8);

    EXTI_InitStructure.EXTI_Line = EXTI_Line8;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void EXTI9_5_IRQHandler(void)
{

    if (EXTI_GetITStatus(EXTI_Line8) == SET)
    {

        EXTI_ClearITPendingBit(EXTI_Line8);
        if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_8)){
            SystemInit();
            delay_init(); //恢复时钟
            csp_wtd_handle();
            NVIC_SystemReset();
            delay_ms(500);            
        }

    }
}
static void config_sys_into_standby(void)
{

    config_sys_key_as_interrupt(); //set wake up key
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    PWR_EnterSTOPMode(PWR_Regulator_ON, PWR_STOPEntry_WFI);
}

//待机相关代码end

static void lcd_power_on_func(void)
{
    lcd_power_control(lcd_power_on);
    lcd_power_status = lcd_power_on;
    power_led_control(true);
}

static void lcd_power_off_func(void)
{
    lcd_power_control(lcd_power_off);
    lcd_power_status = lcd_power_off;
    power_led_control(false);
}

void lcd_power_control_func(bool sw)
{
    if (sw)
    {
        lcd_power_on_func();
    }
    else
    {
        lcd_power_off_func();
    }
}
lcd_power_status_t get_lcd_power_status(void)
{
    return lcd_power_status;
}

void power_key_press_event_handle(void)
{
    uint8_t i=0;
    uint32_t error_tick=0;
    EXTI_InitTypeDef EXTI_InitStructure;
    if (get_lcd_power_status() == lcd_power_off)
    {
        if (get_tft_com_transmit_sw() == true)
            debug_sender_str("LCD POWRER ON \r\n");
        power_led_control(true);
        lcd_power_on_func();

        //开机的时候不使用外部中断唤醒功能
        EXTI_InitStructure.EXTI_Line = EXTI_Line15;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);
    }
    else
    {
        if (get_tft_com_transmit_sw() == true)
            debug_sender_str("LCD POWRER OFF \r\n");

        power_led_control(false);
        lcd_power_off_func();



        for(i=0;i<5;){
            if(get_box_status(i) == box_running_forward || get_box_status(i) == box_running_backward){
                csp_wtd_handle();
                arg_app_hanlde();
                periph_motor_handle();
                csp_timer_handle();
                error_tick++;
            }else{
                i++;
            }
            if(error_tick > 0xffffff){
                error_tick = 0;//等待很长时间都没有复位，出现error
                break;
            }
            //如果有运动任务，等箱体运动任务完成再关机
        }

        motor_enable_control(false);
        config_sys_into_standby(); //进入休眠模式
    }
}

void periph_power_init(void)
{
    lcd_power_on_func(); //默认打开lcd的电源
    //注册POWR_KEY的事件，目前设置的是下降沿
    register_key_press_event(5, power_key_press_event_handle);
}

void periph_power_handle(void)
{
}
