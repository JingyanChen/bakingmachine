#include <string.h>
#include <stdio.h>
#include "csp_adc.h"
#include "csp_uart.h"
#include "csp_gpio.h"
#include "csp_timer.h"
#include "arg_debug_pro.h"
#include "delay.h"
#include "csp_pwm.h"
#include <stdbool.h>
#include <stdlib.h>
#include "arg_pid.h"
#include "periph_motor.h"
#include "periph_humidity_sys.h"
#include "arg_version.h"

static bool temp_gui_upload_sw  = false;
static bool tft_com_transmit_sw = false;
void arg_debug_pro_init(void){
    temp_gui_upload_sw = false;
    tft_com_transmit_sw = false;
}

bool get_tft_com_transmit_sw(void){
    return tft_com_transmit_sw;
}

static void help(void){
    debug_sender_str("\r\n\r\n\r\n csp test cmd list >>>>>>>\r\n");delay_ms(10);
    debug_sender_str(" 1 get_csp_adc \r\n");delay_ms(10);
    debug_sender_str(" 2 set_warm_pwm id percent Note id : 0-9 percent 0-1000\r\n");delay_ms(10); 
    debug_sender_str(" 3 set_motor_pwm id percent Note id : 0-4 percent 0-1000\r\n");delay_ms(10); 


    debug_sender_str("\r\n\r\n\r\n periph test cmd list >>>>>>>\r\n");delay_ms(10); 
    debug_sender_str(" 4 water_cool_pump_con id sw Note id: 0-4 sw 0/1\r\n");delay_ms(10); 
    debug_sender_str(" 5 water_cool_vavle_con id sw Note id 0-6 sw 0/1\r\n");delay_ms(10);
    debug_sender_str(" 6 get_liquid_feedback \r\n");delay_ms(10);
    debug_sender_str(" 7 humidity_con id sw Note id:0-1 sw 0/1\r\n");delay_ms(10);
    debug_sender_str(" 8 get_motor_limit \r\n");delay_ms(10);
    debug_sender_str(" 9 get_key_in\r\n");delay_ms(10);
    debug_sender_str(" 10 fan_con id sw Note id 0-4 sw 0/1\r\n");delay_ms(10);
    debug_sender_str(" 11 led_con id sw Note id 0 sw 0/1\r\n");delay_ms(10);
    debug_sender_str(" 12 open_all_vavle \r\n");delay_ms(10);

    debug_sender_str("\r\n\r\n\r\n periph2 test cmd list >>>>>>>\r\n");delay_ms(10);
    debug_sender_str(" 13 set_motor id dir speed Note id 0-4 dir 0/1 speed 0-1000\r\n");delay_ms(10);     
    debug_sender_str(" 14 set_acc_motor id dir t Note id 0-4 dir 0/1 t 2000-65535\r\n");delay_ms(10);  
    debug_sender_str(" 15 change_water_all \r\n");delay_ms(10); 
    debug_sender_str(" 16 out_water_all \r\n");delay_ms(10);

    debug_sender_str("\r\n\r\n\r\n app test cmd list >>>>>>>\r\n");delay_ms(10);  
    debug_sender_str(" 17 get_temp \r\n");delay_ms(10);     
    debug_sender_str(" 18 get_pid_sw \r\n");delay_ms(10);   
    debug_sender_str(" 19 start_pid id taget_temp Note id 0-9 target_temp 25 - 100\r\n");delay_ms(10);  
    debug_sender_str(" 20 open_temp_gui  open_temp_gui Note press Enter to Stop\r\n");delay_ms(10);
    debug_sender_str(" 21 version\r\n");delay_ms(10);
    debug_sender_str(" 22 open_tft_com_debug  Note tft com will transmit to debug port\r\n");delay_ms(10);
    debug_sender_str(" 23 close_tft_com_debug  Note close tft com transmit\r\n ");delay_ms(10);
}

static void get_csp_adc(void){
    uint8_t sender_buf[100];
    uint8_t i=0;
    uint16_t mv = 0;

    for(i=0;i<10;i++){
        mv = get_adc_v(i,DEFAULT_REF_MV);
        sprintf((char *)sender_buf,">>> adc %d voltage is %d mV \r\n" , i,mv);
        debug_sender_str(sender_buf);
        delay_ms(10);
    }
}
static void get_temp(void){
    uint8_t sender_buf[100];
    uint8_t i=0;
    uint16_t mv = 0;

    for(i=0;i<10;i++){
        mv = adc_temp_data[i];
        sprintf((char *)sender_buf,">>> temp  %d is %d  / 0.1 degree centigrade \r\n" , i,mv);
        debug_sender_str(sender_buf);
        delay_ms(10);
    }
}
static void set_warm_pwm(void){
    //继续分析数据包，debug_uart_rx_buf,debug_uart_rec_len
    //获得 set_warm_pwm 1 200 0x0d 0x0a 解析出 1 200这两个数据出来
    uint8_t i=0;
    uint8_t k_pos[10];
    uint8_t k=0;
    uint8_t j=0;

    uint8_t pra_str1[20];
    uint8_t pra_str2[20];

    uint8_t send_buf[100];
    uint16_t pra1=0,pra2=0;

    //确定空格符号的位置
    for(i=0;i<debug_uart_rec_len;i++){
        if(debug_uart_rx_buf[i] == ' '){
            k_pos[k] = i;
            k++;
        }
    }

    if(k != 2){
        debug_sender_str("command error\r\n");
        return ;
    }
        
    
    for(i=k_pos[0]+1;i<k_pos[1];i++){
        pra_str1[j] = debug_uart_rx_buf[i];
        if(pra_str1[j] <'0' || pra_str1[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }
        j++;
    }
		pra_str1[j] = '\0';
		
    j=0;

    for(i=k_pos[1]+1;i<debug_uart_rec_len-2;i++){
        pra_str2[j] = debug_uart_rx_buf[i];
        if(pra_str2[j] <'0' || pra_str2[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }        
        j++;
    }    
		pra_str1[j] = '\0';
		
    pra1 = atoi((const char *)pra_str1);
    pra2 = atoi((const char *)pra_str2);

    //pra check 

    if(pra1 > 9){
        debug_sender_str("id pra error ,please input 0-9");
        return ;
    }
        

    if(pra2 > 1000){
        debug_sender_str("percent pra error ,please input 0-1000");
        return ;        
    }

    set_software_pwm(pra1,pra2);

    sprintf((char *)send_buf,"set warmer %d as %d / 1000 PWM success\r\n",pra1,pra2);
    debug_sender_str(send_buf);
}
static void set_motor_pwm(void){
    //继续分析数据包，debug_uart_rx_buf,debug_uart_rec_len
    //获得 set_motor_pwm 1 200 0x0d 0x0a 解析出 1 200这两个数据出来
    uint8_t i=0;
    uint8_t k_pos[10];
    uint8_t k=0;
    uint8_t j=0;

    uint8_t pra_str1[20];
    uint8_t pra_str2[20];

    uint8_t send_buf[100];
    uint16_t pra1=0,pra2=0;

    //确定空格符号的位置
    for(i=0;i<debug_uart_rec_len;i++){
        if(debug_uart_rx_buf[i] == ' '){
            k_pos[k] = i;
            k++;
        }
    }

    if(k != 2){
        debug_sender_str("command error\r\n");
        return ;
    }
        
    
    for(i=k_pos[0]+1;i<k_pos[1];i++){
        pra_str1[j] = debug_uart_rx_buf[i];
        if(pra_str1[j] <'0' || pra_str1[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }
        j++;
    }
		pra_str1[j] = '\0';
    j=0;

    for(i=k_pos[1]+1;i<debug_uart_rec_len-2;i++){
        pra_str2[j] = debug_uart_rx_buf[i];
        if(pra_str2[j] <'0' || pra_str2[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }        
        j++;
    }    
		pra_str2[j] = '\0';
		
    pra1 = atoi((const char *)pra_str1);
    pra2 = atoi((const char *)pra_str2);

    if(pra1 > 4){
        debug_sender_str("id pra error ,please input 0-4");
        return ;
    }
        

    if(pra2 > 1000){
        debug_sender_str("percent pra error ,please input 0-1000");
        return ;        
    }
    set_pwm(pra1 , (float)pra2 / 1000.0);

    sprintf((char *)send_buf,"set motor %d as %d / 1000 PWM success\r\n",pra1,pra2);
    debug_sender_str(send_buf);
}
static void water_cool_pump_con(void){
    //获得 set_motor_pwm 1 200 0x0d 0x0a 解析出 1 200这两个数据出来
    uint8_t i=0;
    uint8_t k_pos[10];
    uint8_t k=0;
    uint8_t j=0;

    uint8_t pra_str1[20];
    uint8_t pra_str2[20];

    uint8_t send_buf[100];
    uint16_t pra1=0,pra2=0;

    //确定空格符号的位置
    for(i=0;i<debug_uart_rec_len;i++){
        if(debug_uart_rx_buf[i] == ' '){
            k_pos[k] = i;
            k++;
        }
    }

    if(k != 2){
        debug_sender_str("command error\r\n");
        return ;
    }
        
    
    for(i=k_pos[0]+1;i<k_pos[1];i++){
        pra_str1[j] = debug_uart_rx_buf[i];
        if(pra_str1[j] <'0' || pra_str1[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }
        j++;
    }
		pra_str1[j] = '\0';
    j=0;

    for(i=k_pos[1]+1;i<debug_uart_rec_len-2;i++){
        pra_str2[j] = debug_uart_rx_buf[i];
        if(pra_str2[j] <'0' || pra_str2[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }        
        j++;
    }    
		pra_str2[j] = '\0';
		
    pra1 = atoi((const char *)pra_str1);
    pra2 = atoi((const char *)pra_str2);

    if(pra1 > 4){
        debug_sender_str("id pra error ,please input 0-4");
        return ;
    }
        

    if(pra2 !=1 && pra2 !=0 ){
        debug_sender_str("sw pra error ,please input 0/1");
        return ;        
    }

    water_cool_pump_control(pra1,pra2);

    sprintf((char *)send_buf,"set pump con %d as %d success\r\n",pra1,pra2);

    debug_sender_str(send_buf);
}
static void water_cool_vavle_con(void){
//继续分析数据包，debug_uart_rx_buf,debug_uart_rec_len
    //获得 set_motor_pwm 1 200 0x0d 0x0a 解析出 1 200这两个数据出来
    uint8_t i=0;
    uint8_t k_pos[10];
    uint8_t k=0;
    uint8_t j=0;

    uint8_t pra_str1[20];
    uint8_t pra_str2[20];

    uint8_t send_buf[100];
    uint16_t pra1=0,pra2=0;

    //确定空格符号的位置
    for(i=0;i<debug_uart_rec_len;i++){
        if(debug_uart_rx_buf[i] == ' '){
            k_pos[k] = i;
            k++;
        }
    }

    if(k != 2){
        debug_sender_str("command error\r\n");
        return ;
    }
        
    
    for(i=k_pos[0]+1;i<k_pos[1];i++){
        pra_str1[j] = debug_uart_rx_buf[i];
        if(pra_str1[j] <'0' || pra_str1[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }
        j++;
    }
		pra_str1[j] = '\0';
    j=0;

    for(i=k_pos[1]+1;i<debug_uart_rec_len-2;i++){
        pra_str2[j] = debug_uart_rx_buf[i];
        if(pra_str2[j] <'0' || pra_str2[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }        
        j++;
    }    
		pra_str2[j] = '\0';
		
    pra1 = atoi((const char *)pra_str1);
    pra2 = atoi((const char *)pra_str2);

    if(pra1 > 6){
        debug_sender_str("id pra error ,please input 0-6");
        return ;
    }
        

    if(pra2 !=1 && pra2 !=0 ){
        debug_sender_str("sw pra error ,please input 0/1");
        return ;        
    }

    water_cool_vavle_control(pra1,pra2);

    sprintf((char *)send_buf,"set vavle con %d as %d success\r\n",pra1,pra2);
    
    debug_sender_str(send_buf);
}
static void get_liquid_feedback(void){
    uint8_t sender_buf[100];
    uint8_t i=0;
    uint16_t sw = 0;

    for(i=0;i<10;i++){
        sw = get_liquid_feedback_v(i);
        if(sw)
            sprintf((char *)sender_buf,">>> liquid %d voltage is true \r\n" , i);
        else
            sprintf((char *)sender_buf,">>> liquid %d voltage is false \r\n" , i);
        debug_sender_str(sender_buf);
        delay_ms(10);
    }
}
static void humidity_con(void){
//获得 set_motor_pwm 1 200 0x0d 0x0a 解析出 1 200这两个数据出来
    uint8_t i=0;
    uint8_t k_pos[10];
    uint8_t k=0;
    uint8_t j=0;

    uint8_t pra_str1[20];
    uint8_t pra_str2[20];

    uint8_t send_buf[100];
    uint16_t pra1=0,pra2=0;

    //确定空格符号的位置
    for(i=0;i<debug_uart_rec_len;i++){
        if(debug_uart_rx_buf[i] == ' '){
            k_pos[k] = i;
            k++;
        }
    }

    if(k != 2){
        debug_sender_str("command error\r\n");
        return ;
    }
        
    
    for(i=k_pos[0]+1;i<k_pos[1];i++){
        pra_str1[j] = debug_uart_rx_buf[i];
        if(pra_str1[j] <'0' || pra_str1[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }
        j++;
    }
		pra_str1[j] = '\0';
    j=0;

    for(i=k_pos[1]+1;i<debug_uart_rec_len-2;i++){
        pra_str2[j] = debug_uart_rx_buf[i];
        if(pra_str2[j] <'0' || pra_str2[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }        
        j++;
    }    
		pra_str2[j] = '\0';
		
    pra1 = atoi((const char *)pra_str1);
    pra2 = atoi((const char *)pra_str2);

    if(pra1 > 1){
        debug_sender_str("id pra error ,please input 0-1");
        return ;
    }
        

    if(pra2 !=1 && pra2 !=0 ){
        debug_sender_str("sw pra error ,please input 0/1");
        return ;        
    }

    humidity_control(pra1,pra2);

    sprintf((char *)send_buf,"set humidity con %d as %d success\r\n",pra1,pra2);

    debug_sender_str(send_buf);
}
static void get_motor_limit(void){
    uint8_t sender_buf[100];
    uint8_t i=0;
    uint16_t sw = 0;

    for(i=0;i<10;i++){
        sw = get_motor_limit_v(i);
        if(sw)
            sprintf((char *)sender_buf,">>> motor limit %d voltage is true \r\n" , i);
        else
            sprintf((char *)sender_buf,">>> motor limit %d voltage is false \r\n" , i);
        debug_sender_str(sender_buf);
        delay_ms(10);
    }
}
static void get_key_in(void){
    uint8_t sender_buf[100];
    uint8_t i=0;
    uint16_t sw = 0;

    for(i=0;i<6;i++){
        sw = get_key_in_v(i);
        if(sw)
            sprintf((char *)sender_buf,">>> key %d voltage is true \r\n" , i);
        else
            sprintf((char *)sender_buf,">>> key %d voltage is false  \r\n" , i);
        debug_sender_str(sender_buf);
        delay_ms(10);
    }
}
static void fan_con(void){
    //获得 set_motor_pwm 1 200 0x0d 0x0a 解析出 1 200这两个数据出来
    uint8_t i=0;
    uint8_t k_pos[10];
    uint8_t k=0;
    uint8_t j=0;

    uint8_t pra_str1[20];
    uint8_t pra_str2[20];

    uint8_t send_buf[100];
    uint16_t pra1=0,pra2=0;

    //确定空格符号的位置
    for(i=0;i<debug_uart_rec_len;i++){
        if(debug_uart_rx_buf[i] == ' '){
            k_pos[k] = i;
            k++;
        }
    }

    if(k != 2){
        debug_sender_str("command error\r\n");
        return ;
    }
        
    
    for(i=k_pos[0]+1;i<k_pos[1];i++){
        pra_str1[j] = debug_uart_rx_buf[i];
        if(pra_str1[j] <'0' || pra_str1[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }
        j++;
    }
		pra_str1[j] = '\0';
    j=0;

    for(i=k_pos[1]+1;i<debug_uart_rec_len-2;i++){
        pra_str2[j] = debug_uart_rx_buf[i];
        if(pra_str2[j] <'0' || pra_str2[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }        
        j++;
    }    
		pra_str2[j] = '\0';
		
    pra1 = atoi((const char *)pra_str1);
    pra2 = atoi((const char *)pra_str2);

    if(pra1 > 4){
        debug_sender_str("id pra error ,please input 0-4");
        return ;
    }
        

    if(pra2 !=1 && pra2 !=0 ){
        debug_sender_str("sw pra error ,please input 0/1");
        return ;        
    }

    fan_control(pra1,pra2);

    sprintf((char *)send_buf,"set fan con %d as %d success\r\n",pra1,pra2);

    debug_sender_str(send_buf);
}
static void led_con(void){
    //获得 set_motor_pwm 1 200 0x0d 0x0a 解析出 1 200这两个数据出来
    uint8_t i=0;
    uint8_t k_pos[10];
    uint8_t k=0;
    uint8_t j=0;

    uint8_t pra_str1[20];
    uint8_t pra_str2[20];

    uint8_t send_buf[100];
    uint16_t pra1=0,pra2=0;

    //确定空格符号的位置
    for(i=0;i<debug_uart_rec_len;i++){
        if(debug_uart_rx_buf[i] == ' '){
            k_pos[k] = i;
            k++;
        }
    }

    if(k != 2){
        debug_sender_str("command error\r\n");
        return ;
    }
        
    
    for(i=k_pos[0]+1;i<k_pos[1];i++){
        pra_str1[j] = debug_uart_rx_buf[i];
        if(pra_str1[j] <'0' || pra_str1[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }
        j++;
    }
		pra_str1[j] = '\0';
    j=0;

    for(i=k_pos[1]+1;i<debug_uart_rec_len-2;i++){
        pra_str2[j] = debug_uart_rx_buf[i];
        if(pra_str2[j] <'0' || pra_str2[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }        
        j++;
    }    
		pra_str2[j] = '\0';
		
    pra1 = atoi((const char *)pra_str1);
    pra2 = atoi((const char *)pra_str2);

    if(pra1 > 0){
        debug_sender_str("id pra error ,please input 0");
        return ;
    }
        

    if(pra2 !=1 && pra2 !=0 ){
        debug_sender_str("sw pra error ,please input 0/1");
        return ;        
    }
    if(pra2 == 1)
        power_led_control(true);
    else
        power_led_control(false);

    sprintf((char *)send_buf,"set led %d as %d success\r\n",pra1,pra2);

    debug_sender_str(send_buf);
}
static void get_pid_sw(void){
    uint8_t sender_buf[100];
    uint8_t i=0;

    for(i=0;i<10;i++){
        if(get_pid_con_sw(i))
            sprintf((char *)sender_buf,">>> pid controller %d is on \r\n" , i);
        else
            sprintf((char *)sender_buf,">>> pid controller %d is off \r\n" ,  i);
        debug_sender_str(sender_buf);
        delay_ms(10);
    }    
}
static void start_pid(void){
    //继续分析数据包，debug_uart_rx_buf,debug_uart_rec_len
    //获得 set_warm_pwm 1 200 0x0d 0x0a 解析出 1 200这两个数据出来
    uint8_t i=0;
    uint8_t k_pos[10];
    uint8_t k=0;
    uint8_t j=0;

    uint8_t pra_str1[20];
    uint8_t pra_str2[20];

    uint8_t send_buf[100];
    uint16_t pra1=0,pra2=0;

    //确定空格符号的位置
    for(i=0;i<debug_uart_rec_len;i++){
        if(debug_uart_rx_buf[i] == ' '){
            k_pos[k] = i;
            k++;
        }
    }

    if(k != 2){
        debug_sender_str("command error\r\n");
        return ;
    }
        
    
    for(i=k_pos[0]+1;i<k_pos[1];i++){
        pra_str1[j] = debug_uart_rx_buf[i];
        if(pra_str1[j] <'0' || pra_str1[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }
        j++;
    }
		pra_str1[j] = '\0';
		
    j=0;

    for(i=k_pos[1]+1;i<debug_uart_rec_len-2;i++){
        pra_str2[j] = debug_uart_rx_buf[i];
        if(pra_str2[j] <'0' || pra_str2[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }        
        j++;
    }    
		pra_str1[j] = '\0';
		
    pra1 = atoi((const char *)pra_str1);
    pra2 = atoi((const char *)pra_str2);

    //pra check 

    if(pra1 > 9){
        debug_sender_str("id pra error ,please input 0-9");
        return ;
    }
        
    //todo
    if(pra2 > 1000 || pra2 < 250){
        debug_sender_str("taget temp error. vaild temp is 25-100");
        return ;        
    }

    //打开PID 算法

    start_pid_controller_as_target_temp(pra1,pra2);

    sprintf((char *)send_buf,"start pid controller %d target %d is success\r\n",pra1,pra2);
    debug_sender_str(send_buf);    
}
static void set_motor(void){
    //继续分析数据包，debug_uart_rx_buf,debug_uart_rec_len
    //获得 set_motor_pwm 1 200 0x0d 0x0a 解析出 1 200这两个数据出来
    uint8_t i=0;
    uint8_t k_pos[10];
    uint8_t k=0;
    uint8_t j=0;

    uint8_t pra_str1[20];
    uint8_t pra_str2[20];
    uint8_t pra_str3[20];

    uint8_t send_buf[100];
    uint16_t pra1=0,pra2=0,pra3=0;

    //确定空格符号的位置
    for(i=0;i<debug_uart_rec_len;i++){
        if(debug_uart_rx_buf[i] == ' '){
            k_pos[k] = i;
            k++;
        }
    }

    if(k != 3){
        debug_sender_str("command error\r\n");
        return ;
    }
        
    
    for(i=k_pos[0]+1;i<k_pos[1];i++){
        pra_str1[j] = debug_uart_rx_buf[i];
        if(pra_str1[j] <'0' || pra_str1[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }
        j++;
    }
		pra_str1[j] = '\0';
    j=0;

    for(i=k_pos[1]+1;i<k_pos[2];i++){
        pra_str2[j] = debug_uart_rx_buf[i];
        if(pra_str2[j] <'0' || pra_str2[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }        
        j++;
    }    
		pra_str2[j] = '\0';


    j=0;

    for(i=k_pos[2]+1;i<debug_uart_rec_len-2;i++){
        pra_str3[j] = debug_uart_rx_buf[i];
        if(pra_str3[j] <'0' || pra_str3[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }        
        j++;
    }    
		pra_str3[j] = '\0';

    pra1 = atoi((const char *)pra_str1);
    pra2 = atoi((const char *)pra_str2);
    pra3 = atoi((const char *)pra_str3);

    if(pra1 > 4){
        debug_sender_str("id pra error ,please input 0-4");
        return ;
    }
        

    if(pra2 != 1 && pra2 !=0){
        debug_sender_str("dir error ,please input 0/1");
        return ;        
    }

    if(pra3 > 1000){
        debug_sender_str("percent pra error ,please input 0-1000");
        return ;        
    }

    set_motor_speed_dir(pra1,(dir_t)pra2, pra3);

    sprintf((char *)send_buf,"set motor %d dir %d percent %d/ 1000 PWM success\r\n",pra1,pra2,pra3);
    debug_sender_str(send_buf);
}
static void set_acc_motor(void){
//继续分析数据包，debug_uart_rx_buf,debug_uart_rec_len
    //获得 set_motor_pwm 1 200 0x0d 0x0a 解析出 1 200这两个数据出来
    uint8_t i=0;
    uint8_t k_pos[10];
    uint8_t k=0;
    uint8_t j=0;

    uint8_t pra_str1[20];
    uint8_t pra_str2[20];
    uint8_t pra_str3[20];

    uint8_t send_buf[100];
    uint16_t pra1=0,pra2=0,pra3=0;

    //确定空格符号的位置
    for(i=0;i<debug_uart_rec_len;i++){
        if(debug_uart_rx_buf[i] == ' '){
            k_pos[k] = i;
            k++;
        }
    }

    if(k != 3){
        debug_sender_str("command error\r\n");
        return ;
    }
        
    
    for(i=k_pos[0]+1;i<k_pos[1];i++){
        pra_str1[j] = debug_uart_rx_buf[i];
        if(pra_str1[j] <'0' || pra_str1[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }
        j++;
    }
		pra_str1[j] = '\0';
    j=0;

    for(i=k_pos[1]+1;i<k_pos[2];i++){
        pra_str2[j] = debug_uart_rx_buf[i];
        if(pra_str2[j] <'0' || pra_str2[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }        
        j++;
    }    
		pra_str2[j] = '\0';


    j=0;

    for(i=k_pos[2]+1;i<debug_uart_rec_len-2;i++){
        pra_str3[j] = debug_uart_rx_buf[i];
        if(pra_str3[j] <'0' || pra_str3[j] >'9'){
            debug_sender_str("command error\r\n");
            return ;            
        }        
        j++;
    }    
		pra_str3[j] = '\0';

    pra1 = atoi((const char *)pra_str1);
    pra2 = atoi((const char *)pra_str2);
    pra3 = atoi((const char *)pra_str3);

    if(pra1 > 4){
        debug_sender_str("id pra error ,please input 0-4");
        return ;
    }
        

    if(pra2 != 1 && pra2 !=0){
        debug_sender_str("dir error ,please input 0/1");
        return ;        
    }

    if(pra3 < 2000){
        debug_sender_str("tim pra error ,please input 2000 - 65535");
        return ;        
    }

    start_motor_acc_arg(pra1,(dir_t)pra2, pra3);

    sprintf((char *)send_buf,"set motor acc %d dir %d tim %d ms success\r\n",pra1,pra2,pra3);
    debug_sender_str(send_buf);    
}
//源源不断的发送十个温度片的数据
static void open_temp_gui(void){
    temp_gui_upload_sw = true;
    debug_sender_str("temp gui is start...\r\n");
}
static void get_humidity_status(void){

    switch(get_water_injection_status()){
       case no_injection_task :   debug_sender_str("Now status : no_injection_task\r\n");break;
       case change_water_out_status:  debug_sender_str("Now status : change_water_out_status\r\n");break;
       case change_water_in_status:   debug_sender_str("Now status : change_water_in_status\r\n");break;
       case change_water_done:       debug_sender_str("Now status : change_water_done\r\n");break;
       case out_water_status:       debug_sender_str("Now status : out_water_status\r\n");break;
       case out_water_done:         debug_sender_str("Now status : out_water_done\r\n");break;
       default : break;
    }

}
static void version(void){
    uint8_t version_buf[150];

    if(IS_RELEASE == 0)
        sprintf((char*)version_buf,"V%d.%d make_time:%s_%s Debug Verion By Comegene Jingyan Chen",MAIN_VERSION,SECOND_VERSION, __DATE__, __TIME__);
    else
        sprintf((char*)version_buf,"V%d.%d make_time:%s_%s release Verion By Comegene Jingyan Chen",MAIN_VERSION,SECOND_VERSION, __DATE__, __TIME__);

    debug_sender_str(version_buf);
}
static void open_all_vavle(void){
    uint8_t i=0;
    for(i=0;i<7;i++){
        water_cool_vavle_control(i,false);
    }
}
static void change_water_all(void){
    change_water(0xff);
    debug_sender_str("change water success\r\n");
}
static void out_water_all(void){
    out_water(0xff);
    debug_sender_str("out water success\r\n");
}
static void open_tft_com_debug(void){
    tft_com_transmit_sw = true;
    debug_sender_str("tft com transmit open success\r\n");
}

static void close_tft_com_debug(void){
    tft_com_transmit_sw = false;
    debug_sender_str("tft com transmit close success\r\n");
}
debug_func_list_t debug_func_list[] = {

    {help,"help"},{help,"?"},{help,"HELP"},

    {get_csp_adc,"get_csp_adc"},{get_csp_adc,"1"},
    {set_warm_pwm,"set_warm_pwm"},{set_warm_pwm,"2"},
    {set_motor_pwm,"set_motor_pwm"},{set_motor_pwm,"3"},

    {water_cool_pump_con,"water_cool_pump_con"},{water_cool_pump_con,"4"},
    {water_cool_vavle_con,"water_cool_vavle_con"},{water_cool_vavle_con,"5"},
    {get_liquid_feedback,"get_liquid_feedback"},{get_liquid_feedback,"6"},
    {humidity_con,"humidity_con"},{humidity_con,"7"},
    {get_motor_limit,"get_motor_limit"},{get_motor_limit,"8"},
    {get_key_in,"get_key_in"},{get_key_in,"9"},
    {fan_con,"fan_con"},{fan_con,"10"},
    {led_con,"led_con"},{led_con,"11"},
    {open_all_vavle,"open_all_vavle"},{open_all_vavle,"12"},

    {set_motor,"set_motor"},{set_motor,"13"},
    {set_acc_motor,"set_acc_motor"},{set_acc_motor,"14"},
    {change_water_all,"change_water_all"},{change_water_all,"15"},
    {out_water_all,"out_water_all"},{out_water_all,"16"},
    {get_humidity_status ,"get_humidity_status"},{get_humidity_status,"17"},

    {get_temp,"get_temp"},{get_temp,"18"},
    {get_pid_sw,"get_pid_sw"},{get_pid_sw,"19"},
    {start_pid,"start_pid"},{start_pid,"20"},
    {open_temp_gui,"open_temp_gui"},{open_temp_gui,"21"},
    {version,"version"},{version,"1"},
    {open_tft_com_debug,"open_tft_com_debug"},{open_tft_com_debug,"22"},
    {close_tft_com_debug,"close_tft_com_debug"},{close_tft_com_debug,"23"},

    //对没有参数的函数进行快速访问

};


static void arg_debug_packet_decode(uint8_t * buf , uint16_t len){
    uint8_t cmd_buf[50];
    uint8_t i=0;
    //对接受的到的数据包进行解析，从而做出对应的动作

    //如果单单收到的是空格符号，或者只有回车符号，那么返回comegene commander:

    if(len ==  2 && buf[0] == 0x0d && buf[1] == 0x0a){
        debug_sender_str("comegene command:");
        temp_gui_upload_sw = false;//回车关闭一直上报
        return ;
    }

    //判断子集，从而选择指令,从第一个字符，一直到' ' 或者0x0d为止判断完全子集
    //获得有效的数据子集

    for(i=0;i<len;i++){
        if(buf[i] == 0x0d || buf[i] == ' ')
            break;

        cmd_buf[i] = buf[i];
    }
    
    cmd_buf[i] = '\0';

    //获得了有效的指令字符串 cmd_buf 判断是否有与其完全相等的预定字符串

    for(i=0;i<sizeof(debug_func_list)/sizeof(debug_func_list_t);i++){
        if(strcmp((char *)cmd_buf,debug_func_list[i].cmd_str) == 0){
            debug_func_list[i].func();
            return ;
        }
    }

    debug_sender_str("invaild command\r\n");

}

void arg_debug_pro_handle(void){
    uint8_t sender_buf[200];
    if(_UPLOAD_TEMP_GUI_FLAG == false)
        return;
    _UPLOAD_TEMP_GUI_FLAG = false;


    if(debug_buf_is_ready_check()){
        //data is in debug_uart_rx_buf.
        //len is debug_uart_rec_len - 1
        //debug_sender(debug_uart_rx_buf,debug_uart_rec_len);
        arg_debug_packet_decode(debug_uart_rx_buf,debug_uart_rec_len);
        //after used we must clear rx
        clear_debug_uart();
    } 

    if(temp_gui_upload_sw){
        //根据实际需要，上报浮点数，画曲线。
        sprintf((char *)sender_buf,"temp1,%f;temp2,%f;\n",(float)adc_temp_data[0],(float)adc_temp_data[1]);
        //sprintf((char *)sender_buf,"temp1,%f;temp2,%f;temp3,%f;temp4,%f;\n",(float)adc_temp_data[0],(float)adc_temp_data[1],(float)adc_temp_data[2],(float)adc_temp_data[3]);
        debug_sender_str(sender_buf);
    }  
}
