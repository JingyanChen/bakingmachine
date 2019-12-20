#include <stdbool.h>
#include "periph_power.h"
#include "csp_gpio.h"
#include "csp_uart.h"
#include "periph_key.h"
#include "arg_debug_pro.h"
#include "sys.h"
#include "delay.h"

static lcd_power_status_t lcd_power_status;

//待机相关代码

static void config_sys_key_as_interrupt(void)
{

    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource15);

    EXTI_InitStructure.EXTI_Line = EXTI_Line15;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void EXTI15_10_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line15)==SET){                        
        EXTI_ClearITPendingBit(EXTI_Line15); 
        SystemInit();
        delay_init();//恢复时钟                       
    }
}
static void config_sys_into_standby(void)
{

    config_sys_key_as_interrupt(); //set wake up key 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
    PWR_EnterSTOPMode(PWR_Regulator_LowPower,PWR_STOPEntry_WFI);
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
    if (get_lcd_power_status() == lcd_power_off)
    {
        if (get_tft_com_transmit_sw() == true)
            debug_sender_str("LCD POWRER ON \r\n");
        power_led_control(true);
        lcd_power_on_func();
    }
    else
    {
        if (get_tft_com_transmit_sw() == true)
            debug_sender_str("LCD POWRER OFF \r\n");
        power_led_control(false);
        lcd_power_off_func();
        config_sys_into_standby();//进入休眠模式
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
