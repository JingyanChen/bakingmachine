#include <stdbool.h>
#include "periph_power.h"
#include "csp_gpio.h"
#include "csp_uart.h"
#include "periph_key.h"
#include "arg_debug_pro.h"

static lcd_power_status_t lcd_power_status;

static void lcd_power_on_func(void){
    lcd_power_control(lcd_power_on);
    lcd_power_status = lcd_power_on;
    power_led_control(true);
}

static void lcd_power_off_func(void){
    lcd_power_control(lcd_power_off);
    lcd_power_status = lcd_power_off;
    power_led_control(false);
}

void lcd_power_control_func(bool sw){
    if(sw){
        lcd_power_on_func();
    }else{
        lcd_power_off_func();
    }
}
lcd_power_status_t get_lcd_power_status(void){
    return lcd_power_status;
}

static void power_key_press_event_handle(void){
    if(get_lcd_power_status() == lcd_power_off){
        if(get_tft_com_transmit_sw() == true)
            debug_sender_str("LCD POWRER ON \r\n");
        power_led_control(true);
        lcd_power_on_func();
    }else{
        if(get_tft_com_transmit_sw() == true)
            debug_sender_str("LCD POWRER OFF \r\n");
        power_led_control(false); 
        lcd_power_off_func();
    }
}

void periph_power_init(void){
    lcd_power_on_func(); //默认打开lcd的电源
    //注册POWR_KEY的事件，目前设置的是下降沿
    register_key_press_event(5,power_key_press_event_handle);
}

void periph_power_handle(void){

}
