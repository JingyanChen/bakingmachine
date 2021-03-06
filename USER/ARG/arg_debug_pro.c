#include <string.h>
#include <stdio.h>
#include "csp_adc.h"
#include "csp_uart.h"
#include "csp_gpio.h"
#include "csp_timer.h"
#include "arg_debug_pro.h"
#include "csp_pwm.h"
#include "delay.h"
#include "csp_pwm.h"
#include "app.h"
#include <stdbool.h>
#include <stdlib.h>
#include "arg_pid.h"
#include "periph_motor.h"
#include "periph_humidity_sys.h"
#include "arg_version.h"
#include "periph_fan.h"
#include "periph_power.h"
#include "csp_wtd.h"
#include "app.h"
#include "report.h"

static bool temp_gui_upload_sw  = false;
static bool tft_com_transmit_sw = false;
static bool box_running_debug_sw = false;
static bool pid_debug_sw = false;
static bool sys_debug_sw = false;
void arg_debug_pro_init(void){
    uint8_t welcom_string[200];

    temp_gui_upload_sw = false;
    tft_com_transmit_sw = false;
    box_running_debug_sw = false;
    pid_debug_sw = false;
    sys_debug_sw = false;

    sprintf((char *)welcom_string,"\r\nWelcome to comegene debug instruction systems.Version %d.%d.%d [make_time:%s_%s] \r\nType ""help"",""?"",""copyright"" or ""author"" for more information.\r\n",MAIN_VERSION,SECOND_VERSION,IS_RELEASE,__DATE__, __TIME__);
    debug_sender_str(welcom_string);

}
string_decode_t decode_string(uint8_t * str , uint16_t str_len){

    string_decode_t rlt;
    uint16_t i=0,j=0,k=0,end_pos=0;
    uint16_t num_head_pos[MAX_STRING_DECODE_PRA_NUM];
    uint8_t pra_str[MAX_STRING_DECODE_PRA_NUM][20];

    //initial result parameter

    rlt.is_vaild_string = false;
    rlt.pra_num = 0;
    memset(rlt.pra_list, 0, MAX_STRING_DECODE_PRA_NUM);

    if(str_len < 2){
        rlt.is_vaild_string = false;
        return rlt;
    }

    //first check wethere end of 0x0d 0x0a
    if(str[str_len-1] != 0x0a || str[str_len-2] != 0x0d){
        rlt.is_vaild_string = false;
        return rlt;
    }

    //check pra num
    for( i = 0 ; i < str_len - 2 ; i ++){
        if(str[i] == ' '){
            num_head_pos[rlt.pra_num % MAX_STRING_DECODE_PRA_NUM] = i+1;
            rlt.pra_num ++;
        }
    }

    //check pra is '0' - '9' character

    for(j=0;j<rlt.pra_num;j++){

        if(j == rlt.pra_num -1){
            end_pos = str_len-2;
        }else{
            end_pos = num_head_pos[j + 1] - 1;
        }
        for (i = num_head_pos[j]; i <end_pos; i++){
            if(str[i] <'0' || str[i] >'9'){
                rlt.is_vaild_string = false;
                return rlt;                
            }else{
                pra_str[j][k] = str[i];
                k++;
            }
        }
        pra_str[j][k] = '\0';
        k = 0;
    }

    for (j = 0; j < rlt.pra_num; j++){
        rlt.pra_list[j] = atoi((const char *)pra_str[j]);
    }

    rlt.is_vaild_string = true;

    return rlt;      
}
bool get_tft_com_transmit_sw(void){
    return tft_com_transmit_sw;
}

bool get_box_running_debug_sw(void){
    return box_running_debug_sw;
}

bool get_pid_debug_sw(void){
    return pid_debug_sw;
}
static void debug_send_nop(void){
    csp_wtd_handle();
    delay_ms(10);
}

static void copyright(string_decode_t * string_decode_result){
    debug_sender_str("Copyright (c) 2001-2019 Comegene LLC.\r\nAll Rights Reserved.\r\n");
}

static void author(string_decode_t * string_decode_result){
    debug_sender_str("Name:Jingyan Chen\r\nE-mail:xqchendream@163.com\r\nAll Right reserves\r\n");
}
static void help(string_decode_t * string_decode_result){
    debug_sender_str("\r\n\r\n\r\n help cmd list >>>>>>>\r\n");debug_send_nop();
    debug_sender_str(" 1  get_csp_adc \r\n");debug_send_nop();
    debug_sender_str(" 2  set_warm_pwm id percent Note id : 0-9 percent 0-1000\r\n");debug_send_nop();
    debug_sender_str(" 3  set_motor_pwm id percent Note id : 0-4 percent 0-1000\r\n");debug_send_nop();
    debug_sender_str(" 4  water_cool_pump_con id sw Note id: 0-4 sw 0/1\r\n");debug_send_nop();
    debug_sender_str(" 5  water_cool_vavle_con id sw Note id 0-6 sw 0/1\r\n");debug_send_nop();
    debug_sender_str(" 6  get_liquid_feedback \r\n");debug_send_nop();
    debug_sender_str(" 7  humidity_con id sw Note id:0-1 sw 0/1\r\n");debug_send_nop();
    debug_sender_str(" 8  get_motor_limit \r\n");debug_send_nop();
    debug_sender_str(" 9  get_key_in\r\n");debug_send_nop();
    debug_sender_str(" 10 fan_con id sw Note id 0-4 sw 0/1\r\n");debug_send_nop();
    debug_sender_str(" 11 led_con id sw Note id 0 sw 0/1\r\n");debug_send_nop();
    debug_sender_str(" 12 open_all_vavle \r\n");debug_send_nop();
    debug_sender_str(" 13 set_motor id dir speed Note id 0-4 dir 0/1 speed 0-1000\r\n");debug_send_nop();     
    debug_sender_str(" 14 set_acc_motor id dir t Note id 0-4 dir 0/1 t 2000-65535\r\n");debug_send_nop();
    debug_sender_str(" 15 change_water_all \r\n");debug_send_nop();
    debug_sender_str(" 16 out_water_all \r\n");debug_send_nop();
    debug_sender_str(" 17 get_humidity_status \r\n");debug_send_nop();    
    debug_sender_str(" 18 get_temp \r\n");debug_send_nop();     
    debug_sender_str(" 19 get_pid_sw \r\n");debug_send_nop();   
    debug_sender_str(" 20 start_pid id taget_temp Note id 0-9 target_temp 25 - 100\r\n");debug_send_nop();  
    debug_sender_str(" 21 open_temp_gui  open_temp_gui Note press Enter to Stop\r\n");debug_send_nop();
    debug_sender_str(" 22 version\r\n");debug_send_nop();
    debug_sender_str(" 23 open_tft_com_debug  Note tft com will transmit to debug port\r\n");debug_send_nop();
    debug_sender_str(" 24 close_tft_com_debug  Note close tft com transmit\r\n");debug_send_nop();
    debug_sender_str(" 25 read_box_status\r\n");debug_send_nop();
    debug_sender_str(" 26 read_fan_status\r\n");debug_send_nop();
    debug_sender_str(" 27 power_on\r\n");debug_send_nop();  
    debug_sender_str(" 28 power_off\r\n");debug_send_nop();  
    debug_sender_str(" 29 get_power_status\r\n");debug_send_nop();   
    debug_sender_str(" 30 wtd_test\r\n");debug_send_nop();  
    debug_sender_str(" 31 copyright\r\n");debug_send_nop();   
    debug_sender_str(" 32 author\r\n");debug_send_nop();  
    debug_sender_str(" 33 open_box_speed_debug\r\n");debug_send_nop();
    debug_sender_str(" 34 close_box_speed_debug\r\n");debug_send_nop();
    debug_sender_str(" 35 get_box_status_func\r\n");debug_send_nop();
    debug_sender_str(" 36 start_pid_test\r\n");debug_send_nop();
    debug_sender_str(" 37 run_temp_control Note : id 0-4 target_temp 250 - 1000 bool 0/1\r\n");debug_send_nop();
    debug_sender_str(" 38 stop_temp_control Note : id 0-4 target_temp 250 - 1000 bool 0/1\r\n");debug_send_nop();
    debug_sender_str(" 39 get_task_machine_status\r\n");debug_send_nop();
    debug_sender_str(" 40 get_road_status\r\n");debug_send_nop();
    debug_sender_str(" 41 open_pid_debug\r\n");debug_send_nop();
    debug_sender_str(" 42 get_task_sys_bool\r\n");debug_send_nop();
    debug_sender_str(" 43 press_run_key\r\n");debug_send_nop();
    debug_sender_str(" 44 start_down_temp\r\n");debug_send_nop();
    debug_sender_str(" 45 stop_down_temp\r\n");debug_send_nop();
    debug_sender_str(" 46 start_all_fan\r\n");debug_send_nop();
    debug_sender_str(" 47 stop_all_fan\r\n");debug_send_nop();
    debug_sender_str(" 48 press_power_key\r\n");debug_send_nop();
    debug_sender_str(" 49 sys_debug\r\n");debug_send_nop();
    debug_sender_str(" 50 start_one_hdr\r\n");debug_send_nop();
    debug_sender_str(" 51 start_all_hdr\r\n");debug_send_nop();
    debug_sender_str(" 52 start_one_tcp\r\n");debug_send_nop();
    debug_sender_str(" 53 start_all_tcp\r\n");debug_send_nop();    
}

static void get_csp_adc(string_decode_t * string_decode_result){
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
static void get_temp(string_decode_t * string_decode_result){
    uint8_t sender_buf[100];
    uint8_t i=0;
    uint16_t mv = 0;

    for(i=0;i<10;i++){
        mv = get_temp_data(i);
        sprintf((char *)sender_buf,">>> temp  %d is %d  / 0.1 degree centigrade \r\n" , i,mv);
        debug_sender_str(sender_buf);
        delay_ms(10);
    }
}
static void set_warm_pwm(string_decode_t * string_decode_result){
    uint8_t send_buf[100];

    if(string_decode_result->pra_list[0] > 9){
        debug_sender_str("id pra error ,please input 0-9");
        return ;
    }
        

    if(string_decode_result->pra_list[1] > 1000){
        debug_sender_str("percent pra error ,please input 0-1000");
        return ;        
    }

    set_software_pwm(string_decode_result->pra_list[0],string_decode_result->pra_list[1]);

    sprintf((char *)send_buf,"set warmer %d as %d / 1000 PWM success\r\n",string_decode_result->pra_list[0],string_decode_result->pra_list[1]);
    debug_sender_str(send_buf);
}
static void set_motor_pwm(string_decode_t * string_decode_result){
     uint8_t send_buf[100];

    if(string_decode_result->pra_list[0] > 4){
        debug_sender_str("id pra error ,please input 0-4");
        return ;
    }
        

    if(string_decode_result->pra_list[1] > 1000){
        debug_sender_str("percent pra error ,please input 0-1000");
        return ;        
    }
    set_pwm(string_decode_result->pra_list[0] , (float)string_decode_result->pra_list[1] / 1000.0);

    sprintf((char *)send_buf,"set motor %d as %d / 1000 PWM success\r\n",string_decode_result->pra_list[0],string_decode_result->pra_list[1]);
    debug_sender_str(send_buf);
}
static void water_cool_pump_con(string_decode_t * string_decode_result){

    uint8_t send_buf[100];

    if(string_decode_result->pra_list[0] > 4){
        debug_sender_str("id pra error ,please input 0-4");
        return ;
    }
        

    if(string_decode_result->pra_list[1] !=1 && string_decode_result->pra_list[1] !=0 ){
        debug_sender_str("sw pra error ,please input 0/1");
        return ;        
    }

    water_cool_pump_control(string_decode_result->pra_list[0],string_decode_result->pra_list[1]);

    sprintf((char *)send_buf,"set pump con %d as %d success\r\n",string_decode_result->pra_list[0],string_decode_result->pra_list[1]);

    debug_sender_str(send_buf);
}
static void water_cool_vavle_con(string_decode_t * string_decode_result){
    uint8_t send_buf[100];

    if(string_decode_result->pra_list[0] > 6){
        debug_sender_str("id pra error ,please input 0-6");
        return ;
    }
        

    if(string_decode_result->pra_list[0] !=1 && string_decode_result->pra_list[1] !=0 ){
        debug_sender_str("sw pra error ,please input 0/1");
        return ;        
    }

    water_cool_vavle_control(string_decode_result->pra_list[0],string_decode_result->pra_list[1]);

    sprintf((char *)send_buf,"set vavle con %d as %d success\r\n",string_decode_result->pra_list[0],string_decode_result->pra_list[1]);
    
    debug_sender_str(send_buf);
}
static void get_liquid_feedback(string_decode_t * string_decode_result){
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
static void humidity_con(string_decode_t * string_decode_result){
    uint8_t send_buf[100];
    if(string_decode_result->pra_list[0] > 1){
        debug_sender_str("id pra error ,please input 0-1");
        return ;
    }
        

    if(string_decode_result->pra_list[1] !=1 && string_decode_result->pra_list[1] !=0 ){
        debug_sender_str("sw pra error ,please input 0/1");
        return ;        
    }

    humidity_control(string_decode_result->pra_list[0],string_decode_result->pra_list[1]);

    sprintf((char *)send_buf,"set humidity con %d as %d success\r\n",string_decode_result->pra_list[0],string_decode_result->pra_list[1]);

    debug_sender_str(send_buf);
}
static void get_motor_limit(string_decode_t * string_decode_result){
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
static void get_key_in(string_decode_t * string_decode_result){
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
static void fan_con(string_decode_t * string_decode_result){

    uint8_t send_buf[100];
    if(string_decode_result->pra_list[0] > 4){
        debug_sender_str("id pra error ,please input 0-4");
        return ;
    }
        

    if(string_decode_result->pra_list[1] !=1 && string_decode_result->pra_list[1] !=0 ){
        debug_sender_str("sw pra error ,please input 0/1");
        return ;        
    }

    set_fan(string_decode_result->pra_list[0],string_decode_result->pra_list[1]);

    sprintf((char *)send_buf,"set fan con %d as %d success\r\n",string_decode_result->pra_list[0],string_decode_result->pra_list[1]);

    debug_sender_str(send_buf);
}
static void led_con(string_decode_t * string_decode_result){
    
    uint8_t send_buf[100];
    
    if(string_decode_result->pra_list[0] > 0){
        debug_sender_str("id pra error ,please input 0");
        return ;
    }
        

    if(string_decode_result->pra_list[1] !=1 && string_decode_result->pra_list[1] !=0 ){
        debug_sender_str("sw pra error ,please input 0/1");
        return ;        
    }
    if(string_decode_result->pra_list[1] == 1)
        power_led_control(true);
    else
        power_led_control(false);

    sprintf((char *)send_buf,"set led %d as %d success\r\n",string_decode_result->pra_list[0],string_decode_result->pra_list[1]);

    debug_sender_str(send_buf);
}
static void get_pid_sw(string_decode_t * string_decode_result){
    uint8_t sender_buf[100];
    uint8_t i=0;

    for(i=0;i<10;i++){
        if(get_pid_con_sw(i))
            sprintf((char *)sender_buf,">>> pid controller %d is on target temp %d \r\n" , i,get_pid_con_sw(i));
        else
            sprintf((char *)sender_buf,">>> pid controller %d is off target temp %d\r\n" ,  i,get_pid_con_sw(i));
        debug_sender_str(sender_buf);
        delay_ms(10);
    }    
}
static void start_pid(string_decode_t * string_decode_result){

    if(string_decode_result->pra_list[0] > 9){
        debug_sender_str("id pra error ,please input 0-9");
        return ;
    }
        
    //todo
    if(string_decode_result->pra_list[1] > 1000 || string_decode_result->pra_list[1] < 250){
        debug_sender_str("taget temp error. vaild temp is 25-100");
        return ;        
    }

    //打开PID 算法

    //start_pid_controller_as_target_temp(pra1,pra2);
    debug_sender_str("this cmd is not supported after version V1.0\r\n");  
    //sprintf((char *)send_buf,"start pid controller %d target %d is success\r\n",pra1,pra2);
    //debug_sender_str(send_buf);    
}
static void set_motor(string_decode_t * string_decode_result){

    uint8_t send_buf[100];

    if(string_decode_result->pra_list[0] > 4){
        debug_sender_str("id pra error ,please input 0-4");
        return ;
    }
        

    if(string_decode_result->pra_list[1] != 1 && string_decode_result->pra_list[1] !=0){
        debug_sender_str("dir error ,please input 0/1");
        return ;        
    }

    if(string_decode_result->pra_list[2] > 1000){
        debug_sender_str("percent pra error ,please input 0-1000");
        return ;        
    }

    set_motor_speed_dir(string_decode_result->pra_list[0],(dir_t)string_decode_result->pra_list[1], string_decode_result->pra_list[2]);

    sprintf((char *)send_buf,"set motor %d dir %d percent %d/ 1000 PWM success\r\n",string_decode_result->pra_list[0],string_decode_result->pra_list[1],string_decode_result->pra_list[2]);
    debug_sender_str(send_buf);
}
static void set_acc_motor(string_decode_t * string_decode_result){

    uint8_t send_buf[100];
    
    if(string_decode_result->pra_list[0] > 4){
        debug_sender_str("id pra error ,please input 0-4");
        return ;
    }
        

    if(string_decode_result->pra_list[1] != 1 && string_decode_result->pra_list[1] !=0){
        debug_sender_str("dir error ,please input 0/1");
        return ;        
    }

    if(string_decode_result->pra_list[2] < 2000){
        debug_sender_str("tim pra error ,please input 2000 - 65535");
        return ;        
    }

    start_motor_acc_arg(string_decode_result->pra_list[0],(dir_t)string_decode_result->pra_list[1], string_decode_result->pra_list[2]);

    sprintf((char *)send_buf,"set motor acc %d dir %d tim %d ms success\r\n",string_decode_result->pra_list[0],string_decode_result->pra_list[1],string_decode_result->pra_list[2]);
    debug_sender_str(send_buf);    
}
static void open_temp_gui(string_decode_t * string_decode_result){
    temp_gui_upload_sw = true;
    debug_sender_str("temp gui is start...\r\n");
}
static void get_humidity_status(string_decode_t * string_decode_result){

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
static void version(string_decode_t * string_decode_result){
    uint8_t version_buf[150];

    if(IS_RELEASE == 0)
        sprintf((char*)version_buf,"V%d.%d make_time:%s_%s Debug Verion By Comegene Jingyan Chen",MAIN_VERSION,SECOND_VERSION, __DATE__, __TIME__);
    else
        sprintf((char*)version_buf,"V%d.%d make_time:%s_%s release Verion By Comegene Jingyan Chen",MAIN_VERSION,SECOND_VERSION, __DATE__, __TIME__);

    debug_sender_str(version_buf);
}
static void open_all_vavle(string_decode_t * string_decode_result){
    uint8_t i=0;
    for(i=0;i<7;i++){
        water_cool_vavle_control(i,false);
    }
}
static void change_water_all(string_decode_t * string_decode_result){
    change_water(0xff);
    debug_sender_str("change water success\r\n");
}
static void out_water_all(string_decode_t * string_decode_result){
    out_water(0xff);
    debug_sender_str("out water success\r\n");
}
static void open_tft_com_debug(string_decode_t * string_decode_result){
    tft_com_transmit_sw = true;
    debug_sender_str("tft com transmit open success\r\n");
}
static void close_tft_com_debug(string_decode_t * string_decode_result){
    tft_com_transmit_sw = false;
    debug_sender_str("tft com transmit close success\r\n");
}
static void read_box_status(string_decode_t * string_decode_result){
    uint8_t sender_buf[100];
    uint8_t i=0;
    box_status_t status = box_off;

    for(i=0;i<5;i++){
        status = get_box_status(i);
        switch(status){
            case box_off :sprintf((char *)sender_buf,">>> box %d status is box_off  \r\n" , i);break;
            case box_running_forward :sprintf((char *)sender_buf,">>> box %d status is box_running_forward  \r\n" , i);break;
            case box_running_backward :sprintf((char *)sender_buf,">>> box %d status is box_running_backward  \r\n" , i);break;
            case box_on :sprintf((char *)sender_buf,">>> box %d status is box_on  \r\n" , i);break;
            case box_unknown:sprintf((char *)sender_buf,">>> box %d status is box_unknown  \r\n" , i);break;
        }
        debug_sender_str(sender_buf);
        delay_ms(10);
    }
}
static void read_fan_status(string_decode_t * string_decode_result){
    uint8_t sender_buf[100];
    uint8_t i=0;
    uint16_t b = 0;

    for(i=0;i<5;i++){
        b = get_fan_status(i);
        sprintf((char *)sender_buf,">>> fan %d sw is %d  \r\n" , i,b);
        debug_sender_str(sender_buf);
        delay_ms(10);
    }
}
static void power_on(string_decode_t * string_decode_result){
    lcd_power_control_func(true);
    debug_sender_str("power on success\r\n");
}
static void power_off(string_decode_t * string_decode_result){
    lcd_power_control_func(false);
    debug_sender_str("power off success\r\n");
}
static void get_power_status(string_decode_t * string_decode_result){
    if(get_lcd_power_status() ==  lcd_power_off){
        debug_sender_str("lcd_power_off\r\n");
    }else{
        debug_sender_str("lcd_power_on\r\n");
    }
}
static void wtd_test(string_decode_t * string_decode_result){
    debug_sender_str("system will block delay 3 seconds\r\n");
    delay_ms(3000);
}
static void open_box_speed_debug(string_decode_t * string_decode_result){
    box_running_debug_sw = true;
    debug_sender_str("box speed debug open success\r\n");
}
static void close_box_speed_debug(string_decode_t * string_decode_result){
    box_running_debug_sw = false;
    debug_sender_str("box speed debug close success\r\n");
}
static void get_box_status_func(string_decode_t * string_decode_result){
    box_status_t status;
    status = get_box_status(0);

    switch(status){
        case box_off:
            debug_sender_str("now_status :box_off\r\n"); break;
        case box_running_forward:
            debug_sender_str("now_status :box_running_forward\r\n");break;
        case box_running_backward:
             debug_sender_str("now_status :box_running_backward\r\n");break;
        case box_on:
            debug_sender_str("now_status :box_on\r\n");break;
    }

}
static void start_pid_test(string_decode_t * string_decode_result){
}
static void run_temp_control(string_decode_t * string_decode_result){

    uint8_t send_buf[100];

    event_t temp_event;

    bool dequeue_result=false;

    if(string_decode_result->pra_list[0] > 4){
        debug_sender_str("id pra error ,please input 0-4");
        return ;
    }
        

    if(string_decode_result->pra_list[1] > 1000 || string_decode_result->pra_list[1] < 250){
        debug_sender_str("temp error ,please input 250-1000");
        return ;        
    }

    if(string_decode_result->pra_list[2] !=0 && string_decode_result->pra_list[2] !=1){
        debug_sender_str("need_change_water pra error 0/1");
        return ;        
    }

    temp_event.event_type = START_CONTROL_TEMP_EVENT;
    temp_event.road_id = string_decode_result->pra_list[0];
    temp_event.target_temp = string_decode_result->pra_list[1];
    temp_event.need_change_water = string_decode_result->pra_list[2];
    temp_event.task_running_over = false;

    /*
     * 三个条件下的控温任务不会进入队列
     * 
     * 1 小范围降温 & 无换水操作 = 不委托水冷，不进入队列
     * 2 小范围升温 & 无换水操作 = 不进入队列
     * 3 大范围降温 & 无换水操作 = 委托水冷，不进入队列
     * 4 大范围升温 = 进入队列
     * 
     */

    //下述代码是是否需要将温度控制任务压入顺序任务队列的识别代码，识别的原则见ARG_PID.H的核心算法3

    if(temp_event.need_change_water == true){
        //如果需要换水，那没话说必须进入队列一个一个处理
        dequeue_result = enqueue_event(temp_event);

        if(dequeue_result){
            sprintf((char *)send_buf,"dequeue event: START_CONTROL_TEMP_EVENT road_id: %d target_temp: %d change_water %d success\r\n",string_decode_result->pra_list[0],string_decode_result->pra_list[1],string_decode_result->pra_list[2]);
        }else{
            sprintf((char *)send_buf,"dequeue event: START_CONTROL_TEMP_EVENT failed ! queue full\r\n");
        }

        debug_sender_str(send_buf);   

    }else{
        //如果不需要换水，判断是否是降温
        if(get_road_temp(temp_event.road_id) > temp_event.target_temp){
            //降温
            set_temp_control_status(temp_event.road_id,TEMP_CONTROL_SPECIAL_WAIT);
            set_pid_controller_mode_as_decentralize_without_set_mode(temp_event.road_id,temp_event.target_temp);//仅注册分散
            if(get_road_temp(temp_event.road_id) > temp_event.target_temp + SMALL_RANGE_DOWN_TEMP){
                //大范围降温，需要委托水冷线程
                //大范围降温，计算水泵运行DELAY时间
                set_water_pump_delay_tim(temp_event.road_id,(uint16_t)((float)(get_road_temp(temp_event.road_id) - temp_event.target_temp) * WATER_PUMP_DELAY_K + WATER_PUMP_DELAY_B));
                
                set_no_reason_stop_decentralized_pwm_sw(temp_event.road_id,true);//暂时关闭分散控温算法，大范围降温时发生

                start_water_cool(temp_event.road_id,temp_event.target_temp,true);
                sprintf((char *)send_buf,"road %d is big cooling control.cooling dump running delay Tim is %d\r\n",temp_event.road_id,(uint16_t)((float)(get_road_temp(temp_event.road_id) - temp_event.target_temp) * WATER_PUMP_DELAY_K + WATER_PUMP_DELAY_B));
                debug_sender_str(send_buf); 
            }else{
                //小范围的降温，依靠自然冷却+分散温控
                sprintf((char *)send_buf,"road %d is small cooling control.cooling dump stop\r\n",temp_event.road_id);
                debug_sender_str(send_buf); 
            }

        }else{
            //升温
            if(get_road_temp(temp_event.road_id) + SMALL_RANGE_UP_TEMP < temp_event.target_temp){
                //大范围升温，压入队列慢慢处理
                dequeue_result = enqueue_event(temp_event);

                if(dequeue_result){
                    sprintf((char *)send_buf,"dequeue event: START_CONTROL_TEMP_EVENT road_id: %d target_temp: %d change_water %d success\r\n",string_decode_result->pra_list[0],string_decode_result->pra_list[1],string_decode_result->pra_list[2]);
                }else{
                    sprintf((char *)send_buf,"dequeue event: START_CONTROL_TEMP_EVENT failed ! queue full\r\n");
                }

                debug_sender_str(send_buf);   
            }else{
                //小范围的升温，单单分散温控处理
                clear_water_cool_logic_machine(temp_event.road_id);
                set_pid_controller_mode_as_decentralize_without_set_mode(temp_event.road_id,temp_event.target_temp);
                set_temp_control_status(temp_event.road_id,TEMP_CONTROL_ALL_READY);

                sprintf((char *)send_buf,"road %d is small heating control.opearate done\r\n",temp_event.road_id);
                debug_sender_str(send_buf); 
            }            
        }
        
    }
    

}
static void stop_temp_control(string_decode_t * string_decode_result){

    uint8_t send_buf[100];

    event_t temp_event;
    event_t now_running_e;

    if(string_decode_result->pra_list[0] > 4){
        debug_sender_str("id pra error ,please input 0-4");
        return ;
    }
        

    temp_event.event_type = STOP_CONTROL_TEMP_EVEN;
    temp_event.road_id = string_decode_result->pra_list[0];
    temp_event.target_temp = string_decode_result->pra_list[1];
    temp_event.need_change_water = string_decode_result->pra_list[2];
    temp_event.task_running_over = false;

    //dequeue_result = enqueue_event(temp_event);
    
    /*
     * 停止操作不再会压入队列，而是做如下两件事
     * 1 立刻无理由关闭当前的温控开关
     * 2 检查当前的运行任务是否是road_id 如果是立刻结束当前的任务
     */

    no_reason_stop_temp_control(temp_event.road_id);//无条件关闭温控
    stop_water_cool(temp_event.road_id);//无条件关闭水泵
    set_temp_control_status(temp_event.road_id,TEMP_CONTORL_STOP);


    if( get_now_running_event_task().road_id == temp_event.road_id){
        now_running_e.task_running_over = true;
        set_now_running_event_task(now_running_e);
        queue_task_handle();
    }else{
        set_task_stop_data(get_now_running_event_task().road_id,true);//委托将来执行任务的时候杀死任务
    }

    //判断需要停止的路是否处于超过70摄氏度的高温，如果是则打开水冷到常温的任务

    if(get_road_temp(temp_event.road_id) > 700){
        start_water_cool(temp_event.road_id,400,false);
    }

    //end

    sprintf((char *)send_buf,"stop road_id: %d success\r\n",temp_event.road_id);
    debug_sender_str(send_buf);       
}
static void print_task_nop(){
    csp_wtd_handle();
    delay_ms(5);
}
static void print_task_info(uint8_t event_index , bool is_running_task ,event_t * e){
    uint8_t debug_sender_buf[100];

    if(is_running_task){
        debug_sender_str("\r\n-----Running Task-----\r\n");
        print_task_nop();
    }else{
        sprintf((char*)debug_sender_buf,"\r\n-----list %d-----\r\n",event_index);
        debug_sender_str(debug_sender_buf);
        print_task_nop();
    }

    if(e->event_type == START_CONTROL_TEMP_EVENT){
        debug_sender_str("Task Type: START_CONTROL_TEMP_EVENT\r\n");
    }else{
        debug_sender_str("Task Type: STOP_CONTROL_TEMP_EVEN\r\n");
    }
    print_task_nop();

    sprintf((char*)debug_sender_buf,"Operate Road Id : %d\r\n" , e->road_id);
    debug_sender_str(debug_sender_buf);
    print_task_nop();

    if(e->event_type == START_CONTROL_TEMP_EVENT){
        sprintf((char*)debug_sender_buf,"Target Temputure: %d\r\n",e->target_temp);
        debug_sender_str(debug_sender_buf);
        print_task_nop();
    }


    sprintf((char*)debug_sender_buf,"Water Operate Sw: %d\r\n",e->need_change_water);
    debug_sender_str(debug_sender_buf);
    print_task_nop();

    debug_sender_str("----------");
    
}
static void get_task_machine_status_debug(string_decode_t * string_decode_result){
    //获得当前的运行状态，哪些被执行，哪些没被执行当前处于什么状态
    //这个函数是算法核心

    /*
     * 此DEBUG的作用指示当前的任务运行管理状态
     * 包括，目前已经接收了哪些任务，当前正在执行的是什么任务
     * 
     */
    uint16_t all_task_num=0;
    event_t now_running_task;
    event_t list_task;
    uint8_t i=0;
    
    all_task_num = get_queue_size();
    now_running_task = get_now_running_event_task();

    if(all_task_num == 0 && get_task_machine_status() == task_machine_idle){
        debug_sender_str("no task running\r\n");
        return ;
    }

    print_task_info(0,true,&now_running_task);

    for(i=0;i<all_task_num;i++){
        list_task = get_pos_queue_ele(i);
        print_task_info(i,false,&list_task);  
    }

}
void get_road_status(string_decode_t * string_decode_result){
    uint8_t i =0;
    uint8_t send_buf[50];
    for(i=0;i<5;i++){
        sprintf((char *)send_buf,">>> %d road status :",i);
        debug_sender_str(send_buf);
        print_task_nop();

        switch(get_temp_control_status(i)){
            case TEMP_CONTORL_STOP:debug_sender_str("TEMP_CONTORL_STOP\r\n");print_task_nop();break;
            case TEMP_CONTROL_UP_DOWN_QUICK_STATUS:debug_sender_str("TEMP_CONTROL_UP_DOWN_QUICK_STATUS\r\n");print_task_nop();break;
            case TEMP_CONTROL_CONSTANT:debug_sender_str("TEMP_CONTROL_CONSTANT\r\n");print_task_nop();break;
            case TEMP_CONTROL_ALL_READY:debug_sender_str("TEMP_CONTROL_ALL_READY\r\n");print_task_nop();break;
            case TEMP_CONTROL_SPECIAL_WAIT:debug_sender_str("TEMP_CONTROL_SPECIAL_WAIT\r\n");print_task_nop();break;
        }
    }
}
void open_pid_debug(string_decode_t * string_decode_result){
    pid_debug_sw = true;
    debug_sender_str("pid debug is start...\r\n");
}
void get_task_sys_bool(string_decode_t * string_decode_result){
    
    uint8_t debug_buf[100];

    bool concen_flag= false;
    bool decen_flag = false;
    bool queue_task_deal_hang_up = false;

    queue_task_deal_hang_up = get_queue_task_deal_hang_up();
    concen_flag = get_concentrate_status();
    decen_flag = get_decentralize_busy_flag();

    sprintf((char *)debug_buf,"concentrate flag : %d\r\ndecentralize flag : %d\r\nget_queue_task_deal_hang_up : %d\r\n",concen_flag,decen_flag,queue_task_deal_hang_up);
    debug_sender_str(debug_buf);
}
void press_run_key(string_decode_t * string_decode_result){
    key_box_logic(0);
}
void start_down_temp(string_decode_t * string_decode_result){
    uint8_t i =0;
    for(i=0;i<5;i++){
        water_cool_pump_control(i,false);
    }
    debug_sender_str("start all water cool pump success\r\n");
}
void stop_down_temp(string_decode_t * string_decode_result){
    uint8_t i =0;
    for(i=0;i<5;i++){
        water_cool_pump_control(i,true);
    }
    debug_sender_str("stop all water cool pump success\r\n");
}
void start_all_fan(string_decode_t * string_decode_result){
    uint8_t i =0;
    for(i=0;i<5;i++){
        fan_control(i,false);
    }
    debug_sender_str("start all fan success\r\n");
}
void stop_all_fan(string_decode_t * string_decode_result){
    uint8_t i =0;
    for(i=0;i<5;i++){
        fan_control(i,true);
    }
    debug_sender_str("stop all fan success\r\n");
}
void press_power_key(string_decode_t * string_decode_result){
    power_key_press_event_handle();
}
void sys_debug(string_decode_t * string_decode_result){
    //比较丰富的方式指示每一层盒子的状态
    //1 指示总体的控温方式 C : 集中控温模式  D ： 分散控温模式
    //2 指示每一路的温度值和目标温度
    //3 指示每一路的水冷泵状态
    //4 指示每一路的逻辑状态

    sys_debug_sw = true;

    debug_sender_str("start sys debug success\r\n");

}
void start_one_hdr(string_decode_t * string_decode_result){
    if(start_HRD_check(false)){
        debug_sender_str("start 0 box HDR test success\r\n");
    }else{
        debug_sender_str("start 0 box HDR test failed\r\n");
    }
    
}
void start_all_hdr(string_decode_t * string_decode_result){
    if(start_HRD_check(true)){
        debug_sender_str("start all box HDR test success\r\n");
    }else{
        debug_sender_str("start all box HDR test success\r\n");
    }
}
debug_func_list_t debug_func_list[] = {

    {help,"help",0},{help,"?",0},{help,"HELP",0},

    {get_csp_adc,"get_csp_adc",0},{get_csp_adc,"1",0},
    {set_warm_pwm,"set_warm_pwm",2},{set_warm_pwm,"2",2},
    {set_motor_pwm,"set_motor_pwm",2},{set_motor_pwm,"3",2},

    {water_cool_pump_con,"water_cool_pump_con",2},{water_cool_pump_con,"4",2},
    {water_cool_vavle_con,"water_cool_vavle_con",2},{water_cool_vavle_con,"5",2},
    {get_liquid_feedback,"get_liquid_feedback",0},{get_liquid_feedback,"6",0},
    {humidity_con,"humidity_con",2},{humidity_con,"7",2},
    {get_motor_limit,"get_motor_limit",0},{get_motor_limit,"8",0},
    {get_key_in,"get_key_in",0},{get_key_in,"9",0},
    {fan_con,"fan_con",2},{fan_con,"10",2},
    {led_con,"led_con",2},{led_con,"11",2},
    {open_all_vavle,"open_all_vavle",0},{open_all_vavle,"12",0},

    {set_motor,"set_motor",3},{set_motor,"13",3},
    {set_acc_motor,"set_acc_motor",3},{set_acc_motor,"14",3},
    {change_water_all,"change_water_all",0},{change_water_all,"15",0},
    {out_water_all,"out_water_all",0},{out_water_all,"16",0},
    {get_humidity_status ,"get_humidity_status",0},{get_humidity_status,"17",0},

    {get_temp,"get_temp",0},{get_temp,"18",0},
    {get_pid_sw,"get_pid_sw",0},{get_pid_sw,"19",0},
    {start_pid,"start_pid",2},{start_pid,"20",2},
    {open_temp_gui,"open_temp_gui",0},{open_temp_gui,"21",0},
    {version,"version",0},{version,"22",0},
    {open_tft_com_debug,"open_tft_com_debug",0},{open_tft_com_debug,"23",0},
    {close_tft_com_debug,"close_tft_com_debug",0},{close_tft_com_debug,"24",0},
    {read_box_status,"read_box_status",0},{read_box_status,"25",0},
    {read_fan_status,"read_fan_status",0},{read_fan_status,"26",0},
    {power_on,"power_on",0},{power_on,"27",0},
    {power_off,"power_off",0},{power_off,"28",0},
    {get_power_status,"get_power_status",0},{get_power_status,"29",0},
    {wtd_test,"wtd_test",0},{wtd_test,"30",0},
    {copyright,"copyright",0},{copyright,"31",0},
    {author,"author",0},{author,"32",0},
    {open_box_speed_debug,"open_box_speed_debug",0},{open_box_speed_debug,"33",0},
    {close_box_speed_debug,"close_box_speed_debug",0},{close_box_speed_debug,"34",0},
    {get_box_status_func,"get_box_status_func",0},{get_box_status_func,"35",0},
    {start_pid_test,"start_pid_test",0},{start_pid_test,"36",0},
    {run_temp_control,"run_temp_control",3},{run_temp_control,"37",3},
    {stop_temp_control,"stop_temp_control",1},{stop_temp_control,"38",1},
    {get_task_machine_status_debug,"get_task_machine_status",0},{get_task_machine_status_debug,"39",0},
    {get_road_status,"get_road_status",0},{get_road_status,"40",0},
    {open_pid_debug,"open_pid_debug",0},{open_pid_debug,"41",0},
    {get_task_sys_bool,"get_task_sys_bool",0},{get_task_sys_bool,"42",0},
    {press_run_key,"press_run_key",0},{press_run_key,"43",0},
    {start_down_temp,"start_down_temp",0},{start_down_temp,"44",0},
    {stop_down_temp,"stop_down_temp",0},{stop_down_temp,"45",0},
    {start_all_fan,"start_all_fan",0},{start_all_fan,"46",0},
    {stop_all_fan,"stop_all_fan",0},{stop_all_fan,"47",0},
    {press_power_key,"press_power_key",0},{press_power_key,"48",0},
    {sys_debug , "sys_debug",0},{sys_debug,"49",0},

    {start_one_hdr , "start_one_hdr",0},{start_one_hdr,"50",0},
    {start_all_hdr , "start_all_hdr",0},{start_all_hdr,"51",0},
  
};


static void arg_debug_packet_decode(uint8_t * buf , uint16_t len){
    uint8_t cmd_buf[50];
    uint8_t i=0;
    string_decode_t string_decode_result;

    //对接受的到的数据包进行解析，从而做出对应的动作

    //如果单单收到的是空格符号，或者只有回车符号，那么返回comegene commander:

    if(len ==  2 && buf[0] == 0x0d && buf[1] == 0x0a){
        debug_sender_str("comegene command:\r\n");
        temp_gui_upload_sw = false;//回车关闭一直上报
        pid_debug_sw = false;
        sys_debug_sw = false;
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
            if(debug_func_list[i].pra_num == 0){
                debug_func_list[i].func(&string_decode_result);
            }else{
                string_decode_result = decode_string(debug_uart_rx_buf,debug_uart_rec_len);
                if(string_decode_result.is_vaild_string == true && string_decode_result.pra_num == debug_func_list[i].pra_num){
                    debug_func_list[i].func(&string_decode_result);
                }else{
                    debug_sender_str("invaild command\r\n"); 
                }
            }
            return ;
        }
    }

    debug_sender_str("invaild command\r\n");

}

void arg_debug_pro_handle(void){
    uint8_t sender_buf[300];
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
        //sprintf((char *)sender_buf,"temp1,%f;temp2,%f;\n",(float)adc_temp_data[0],(float)adc_temp_data[1]);
        sprintf((char *)sender_buf,"temp1,%f;temp2,%f;temp3,%f;temp4,%f;temp5,%f;temp6,%f;temp7,%f;temp8,%f;\n",(float)get_temp_data(0),(float)get_temp_data(1),(float)get_temp_data(2),(float)get_temp_data(3),(float)get_temp_data(4),(float)get_temp_data(5),(float)get_temp_data(6),(float)get_temp_data(7));
        debug_sender_str(sender_buf);
    }  

    if(sys_debug_sw){
    //比较丰富的方式指示每一层盒子的状态
    //1 指示总体的控温方式 C : 集中控温模式  D ： 分散控温模式
    //2 指示每一路的温度值和目标温度
    //3 指示每一路的水冷泵状态
    //4 指示每一路的逻辑状态
    //5 当前控制的road id
    if(get_temp_control_mode() == DECENTRALIZED_CONTROL_MODE){
        debug_sender_str("MODE : D --- ");
        delay_ms(5);
        sprintf((char *)sender_buf,\
        "0=> T: %d/%d W: %d LI: %d pwm (%03d,%03d) | 1=> T: %d/%d W: %d LI: %d pwm (%03d,%03d) | 2=> T: %d/%d W: %d LI: %d pwm (%03d,%03d) | 3=> T: %d/%d W: %d LI: %d pwm (%03d,%03d) | 4=> T: %d/%d W: %d LI: %d pwm (%03d,%03d) --- id :(%d,%d) \r\n"\
        ,get_road_temp(0),get_target_temp(0),get_water_cool_sw(0),get_temp_control_status(0),get_pwm_maker_percent(0),get_pwm_maker_percent(1),\
        get_road_temp(1),get_target_temp(1),get_water_cool_sw(1),get_temp_control_status(1),get_pwm_maker_percent(2),get_pwm_maker_percent(3),\
        get_road_temp(2),get_target_temp(2),get_water_cool_sw(2),get_temp_control_status(2),get_pwm_maker_percent(4),get_pwm_maker_percent(5),\
        get_road_temp(3),get_target_temp(3),get_water_cool_sw(3),get_temp_control_status(3),get_pwm_maker_percent(6),get_pwm_maker_percent(7),\
        get_road_temp(4),get_target_temp(4),get_water_cool_sw(4),get_temp_control_status(4),get_pwm_maker_percent(8),get_pwm_maker_percent(9),\
        get_decentralized_control_road_id(0),get_decentralized_control_road_id(1));

        debug_sender_str(sender_buf);        
    }else{
        debug_sender_str("MODE : C --- ");
        delay_ms(5);
        sprintf((char *)sender_buf,\
        "0=> T: %d/%d W: %d LI: %d pwm (%03d,%03d) | 1=> T: %d/%d W: %d LI: %d pwm (%03d,%03d) | 2=> T: %d/%d W: %d LI: %d pwm (%03d,%03d) | 3=> T: %d/%d W: %d LI: %d pwm (%03d,%03d) | 4=> T: %d/%d W: %d LI: %d pwm (%03d,%03d) --- id :(%d,%d) \r\n"\
        ,get_road_temp(0),get_target_temp(0),get_water_cool_sw(0),get_temp_control_status(0),get_pwm_maker_percent(0),get_pwm_maker_percent(1),\
        get_road_temp(1),get_target_temp(1),get_water_cool_sw(1),get_temp_control_status(1),get_pwm_maker_percent(2),get_pwm_maker_percent(3),\
        get_road_temp(2),get_target_temp(2),get_water_cool_sw(2),get_temp_control_status(2),get_pwm_maker_percent(4),get_pwm_maker_percent(5),\
        get_road_temp(3),get_target_temp(3),get_water_cool_sw(3),get_temp_control_status(3),get_pwm_maker_percent(6),get_pwm_maker_percent(7),\
        get_road_temp(4),get_target_temp(4),get_water_cool_sw(4),get_temp_control_status(4),get_pwm_maker_percent(8),get_pwm_maker_percent(9),\
        get_concentrate_road_id(0),get_concentrate_road_id(1));

        debug_sender_str(sender_buf);
    }


    }
}
