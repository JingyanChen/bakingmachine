#include <string.h>
#include <stdio.h>
#include "csp_adc.h"
#include "csp_uart.h"
#include "arg_debug_pro.h"
#include "delay.h"
#include "csp_pwm.h"
#include <stdbool.h>
#include <stdlib.h>

void arg_debug_pro_init(void){
}


static void help(void){
    debug_sender_str(" get_csp_adc\r\n set_warm_pwm\r\n set_motor_pwm\r\n");   
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
        debug_sender_str("id pra error ,please input 0-5");
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

debug_func_list_t debug_func_list[] = {
    {help,"help"},
    {get_csp_adc,"get_csp_adc"},
    {set_warm_pwm,"set_warm_pwm"},
    {set_motor_pwm,"set_motor_pwm"},
};


static void arg_debug_packet_decode(uint8_t * buf , uint16_t len){
    uint8_t cmd_buf[50];
    uint8_t i=0;
    //对接受的到的数据包进行解析，从而做出对应的动作

    //如果单单收到的是空格符号，或者只有回车符号，那么返回comegene commander:

    if(len ==  2 && buf[0] == 0x0d && buf[1] == 0x0a){
        debug_sender_str("comegene command:");
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
    if(debug_buf_is_ready_check()){
        //data is in debug_uart_rx_buf
        //len is debug_uart_rec_len - 1
        //debug_sender(debug_uart_rx_buf,debug_uart_rec_len);
        arg_debug_packet_decode(debug_uart_rx_buf,debug_uart_rec_len);
        //after used we must clear rx
        clear_debug_uart();
    }       
}
