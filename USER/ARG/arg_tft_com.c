#include "csp_uart.h"
#include "csp_timer.h"
#include "arg_debug_pro.h"
#include "delay.h"
#include "arg_tft_com.h"
#include "app.h"
#include "arg_pid.h"
#include "periph_humidity_sys.h"
#include "periph_fan.h"
#include "csp_adc.h"

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

void arg_tft_com_init(void){
}

static const unsigned char TabH[]={  
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,  
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,  
        0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,  
        0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,  
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,  
        0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,  
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,  
        0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,  
        0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,  
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,  
        0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,  
        0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,  
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,  
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  
        0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,  
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,  
        0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,  
        0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,  
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40  
    };  
static const unsigned char TabL[]={  
        0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,  
        0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,  
        0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,  
        0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,  
        0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,  
        0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,  
        0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,  
        0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,  
        0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,  
        0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,  
        0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,  
        0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,  
        0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,  
        0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,  
        0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,  
        0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,  
        0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,  
        0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,  
        0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,  
        0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,  
        0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,  
        0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,  
        0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,  
        0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,  
        0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,  
        0x43, 0x83, 0x41, 0x81, 0x80, 0x40  
};

 uint16_t lib_calculateCrc16(uint8_t *ptr,  uint8_t len){
    unsigned int index;
    unsigned char crch = 0xFF; 
    unsigned char crcl = 0xFF;  

    while (len--){
        index = crch ^ *ptr++;
        crch = crcl ^ TabH[ index];
        crcl = TabL[ index];
    }
    return ((crcl<<8) | crch);  
}


static void quick_resond_func(uint16_t cmd , uint16_t * load ,uint16_t load_len){
    uint8_t respond_buf[50];
    uint16_t index=0;
    uint16_t crc=0;

    uint8_t i=0;

    if(load_len > LOAD_MAX_LEN)
        return ;
    
    respond_buf[index ++] = 0xee;
    
    respond_buf[index ++] = 0;
    respond_buf[index ++] = load_len + 2;

    respond_buf[index ++] = 0;
    respond_buf[index ++] = cmd;

    for( i=0;i<load_len;i++){
        respond_buf[index ++] = load[i] >> 8;
        respond_buf[index ++] = (uint8_t)load[i];
    }

    
    crc = lib_calculateCrc16(respond_buf+3 , index-3);

    respond_buf[index ++] = crc >> 8;
    respond_buf[index ++] = (uint8_t)crc;

    respond_buf[index ++] = 0xff;
    respond_buf[index ++] = 0xfd;
    respond_buf[index ++] = 0xfe;
    respond_buf[index ++] = 0xff;

    lcd_sender(respond_buf , index);

}


static void open_temp_control_func(tft_mcu_pro_data_t * tft_mcu_pro_data){
    /*
     * 重要API函数，打开温度控制
     * 解析原则: 
     *      load  分布如下
     *      
     *      num     list     temp[n]        bool 
     *      uint16  uint16   uint16 * num   uint16
     */

    uint16_t temp_control_num =0;
    uint16_t temp_list=0;
    uint16_t temp_target[5];
    uint16_t need_change_water_pra;
    uint8_t i=0,j=0,k=0;
    uint8_t debug_buf[100];
    uint16_t respond[50];

    temp_control_t temp_control_pra;

    temp_control_num = tft_mcu_pro_data->load[0];
    temp_control_num <<= 8;
    temp_control_num |=tft_mcu_pro_data->load[1];

    temp_list = tft_mcu_pro_data->load[2];
    temp_list <<= 8;
    temp_list |= tft_mcu_pro_data->load[3]; 

    if(temp_control_num > 5 )
        return ;

    need_change_water_pra = tft_mcu_pro_data->load[4+ temp_control_num * 2]; 
    need_change_water_pra <<= 8;
    need_change_water_pra |= tft_mcu_pro_data->load[5 + temp_control_num * 2]; 

    memset(temp_target,0xff,5);

    for(i=0;i<5;i++){

        if(temp_list & (0x01 << i)){
            j++;

            if(j > temp_control_num)
                break;

            temp_target[i] = tft_mcu_pro_data->load[(4+k) % LOAD_MAX_LEN]; 
            temp_target[i] <<= 8;
            temp_target[i] |= tft_mcu_pro_data->load[(4+k+1) % LOAD_MAX_LEN];

            k+=2; 


            //参数检查，如果出现错误参数，直接返回报错

            if( temp_target[i] > 1000){

                if(get_tft_com_transmit_sw() == true){
                    debug_sender_str("temp error prameter too large\r\n");
                    delay_ms(10);
                }  

                respond[0] = 0x01;
                respond[1] = pra_error;
                quick_resond_func(open_temp_control,respond,2);
                return ;
            } 

            //盒子是否关闭检查，必须关闭以后在做升温操作

            if(get_box_status(i) != box_off){

                if(get_tft_com_transmit_sw() == true){
                    sprintf((char *)debug_buf,"box %d not closed error! \r\n",i);
                    debug_sender_str(debug_buf);
                    delay_ms(10);
                }  

                respond[0] = 0x01;
                respond[1] = box_not_closed;
                quick_resond_func(open_temp_control,respond,2);

                return ;            
        }

        }else{
            temp_target[i] = 0xff;
        }
    }

    temp_control_pra.control_num = (uint8_t)temp_control_num;
    
    //目前按对来处理温控任务
    //0-1 一对  2-3一对 以此类推

    for(i=0;i<5;i++){
        if(temp_target[i] == 0xff){
           temp_control_pra.control_sw[i * 2] = false;
           temp_control_pra.control_sw[i * 2 + 1] = false; 
        }else{
            temp_control_pra.control_sw[i * 2] = true;
            temp_control_pra.control_sw[i * 2 + 1] = true;
            temp_control_pra.control_temp[i * 2] = temp_target[i]; 
            temp_control_pra.control_temp[i * 2 + 1] = temp_target[i]; 

            if(get_tft_com_transmit_sw() == true){
                sprintf((char *)debug_buf,"set %d roads as %d /10 du \r\n",i,temp_target[i]);
                debug_sender_str(debug_buf);
                delay_ms(10);
            }  

        }
    }

    temp_control_pra.need_change_water = (bool)need_change_water_pra;

    if(get_tft_com_transmit_sw() == true){
        if(temp_control_pra.need_change_water == true){
            debug_sender_str("need change water\r\n");
            delay_ms(10);   
        }else{
            debug_sender_str("no need change water\r\n");
            delay_ms(10);               
        }
    }

    //换水操作一定成功，逻辑如下
    /*
     * 如果出现了正在换水，但是又一次触发换水操作，状态机重置
     * 上一次的状态对此操作无影响，此操作一定成功
     */
    config_temp_control_machine(&temp_control_pra);

    if(get_tft_com_transmit_sw() == true){
        debug_sender_str("operate success!\r\n");
        delay_ms(10);
    }    

    respond[0] = 0x00;
    respond[1] = operate_succ;
    quick_resond_func(open_temp_control,respond,2);



}

static void close_temp_control_func(tft_mcu_pro_data_t * tft_mcu_pro_data){
    //关闭所有温控，并且决定是否需要把水抽干
    //直接操作arg_pid里面的操作函数，关闭所有pid控制
    //通过湿度系统开启一次抽水
    //指令格式  operate id 0-4 bool

    uint16_t need_out_water=0;
    uint16_t operate_id = 0;
    uint16_t respond[50];
		uint8_t debug[50];
    bool out_succ = false;

    operate_id = tft_mcu_pro_data->load[0];
    operate_id <<= 8;
    operate_id |=tft_mcu_pro_data->load[1];

    need_out_water = tft_mcu_pro_data->load[2];
    need_out_water <<= 8;
    need_out_water |=tft_mcu_pro_data->load[3];

    if(operate_id > 4){

        if(get_tft_com_transmit_sw() == true){
            debug_sender_str("id pra error\r\n");
            delay_ms(10);
        }  

        respond[0] = 0x01;
        respond[1] = pra_error;
        quick_resond_func(open_temp_control,respond,2);
        return ;
    }

    set_pid_con_sw(operate_id * 2,false);  
    set_pid_con_sw(operate_id * 2 + 1,false);          

    if(get_tft_com_transmit_sw() == true){
        sprintf((char *)debug,"close temp control  %d success!", operate_id);
        debug_sender_str(debug);
        delay_ms(10);
    }  

    if(need_out_water == 1){
        out_succ = out_water(0xff);
        if(get_tft_com_transmit_sw() == true){
            if(out_succ){
                debug_sender_str("start out water success!\r\n");
            }else{
                debug_sender_str("error:Is in the state of water injection into pumping failure, resource conflict \r\n");
            }
            delay_ms(10);
        }  
    }

    if(get_tft_com_transmit_sw() == true){
        debug_sender_str("operate success!\r\n");
        delay_ms(10);
    }    

    respond[0] = 0x00;
    respond[1] = operate_succ;
    quick_resond_func(close_temp_control,respond,2);


}

static void write_fan_func(tft_mcu_pro_data_t * tft_mcu_pro_data){
    //数据格式 id sw
    uint16_t id=0;
    uint16_t sw=0;
    uint16_t respond[50];

    id = tft_mcu_pro_data->load[0];
    id <<= 8;
    id |=tft_mcu_pro_data->load[1];

    sw = tft_mcu_pro_data->load[2];
    sw <<= 8;
    sw |=tft_mcu_pro_data->load[3]; 

    if(id > 4){
        if(get_tft_com_transmit_sw() == true){
            debug_sender_str("id error!\r\n");
            delay_ms(10);
        }    

        respond[0] = 0x01;
        respond[1] = 0x00;
        quick_resond_func(write_fan,respond,2);
         
    }else{
        set_fan(id,sw);

        if(get_tft_com_transmit_sw() == true){
            debug_sender_str("operate success!\r\n");
            delay_ms(10);
        }    

        respond[0] = 0x00;
        respond[1] = operate_succ;
        quick_resond_func(write_fan,respond,2);        
    }  
}

static void read_fan_func(tft_mcu_pro_data_t * tft_mcu_pro_data){
    uint16_t respond[50];
    uint8_t i=0;

    for(i=0;i<5;i++){
       respond[i] = get_fan_status(i);
    }

    quick_resond_func(read_fan,respond,5);  
}

static void write_box_func(tft_mcu_pro_data_t * tft_mcu_pro_data){
    //数据格式 id sw
    uint16_t id=0;
    uint16_t respond[50];

    id = tft_mcu_pro_data->load[0];
    id <<= 8;
    id |=tft_mcu_pro_data->load[1];

    if(id > 4){
        if(get_tft_com_transmit_sw() == true){
            debug_sender_str("id error!\r\n");
            delay_ms(10);
        }    

        respond[0] = 0x01;
        respond[1] = 0x00;
        quick_resond_func(write_box,respond,2);
         
    }else{
        key_box_logic(id);

        if(get_tft_com_transmit_sw() == true){
            debug_sender_str("operate success!\r\n");
            delay_ms(10);
        }    

        respond[0] = 0x00;
        respond[1] = operate_succ;
        quick_resond_func(write_box,respond,2);        
    }   
}

static void read_box_func(tft_mcu_pro_data_t * tft_mcu_pro_data){
    uint16_t respond[50];
    uint8_t i=0;

    for(i=0;i<5;i++){
       respond[i] = get_box_status(i);
    }

    quick_resond_func(read_box,respond,5);     
}

static void read_now_temp_func(tft_mcu_pro_data_t * tft_mcu_pro_data){
    uint16_t respond[50];
    uint8_t i=0;
    uint16_t temp[10];

    //两两一组做平均

    for(i=0;i<10;i++){
        temp[i] = adc_temp_data[i];
    }

    for(i=0;i<5;i++){
       respond[i] = (temp[i * 2] + temp[ i * 2 + 1]) / 2 ;
    }

    quick_resond_func(read_now_temp,respond,5);  
}

static void read_now_temp_target_func(tft_mcu_pro_data_t * tft_mcu_pro_data){
    uint16_t respond[50];
    uint8_t i=0;

    //两两一组做平均

    for(i=0;i<5;i++){
       respond[i] = (tc.control_temp[i * 2] + tc.control_temp[ i * 2 + 1]) / 2 ;
    }

    quick_resond_func(read_now_temp_target,respond,5);    
}


static tft_com_func_list_t tft_com_func_list[]={

    {open_temp_control_func,open_temp_control},
    {close_temp_control_func,close_temp_control},
    {write_fan_func,write_fan},
    {read_fan_func,read_fan},
    {write_box_func,write_box},
    {read_box_func,read_box},
    {read_now_temp_func,read_now_temp},
    {read_now_temp_target_func,read_now_temp_target},

};


static void lcd_tft_cmd_decode(void){
    //对lcd_uart_rx_buf长度为lcd_uart_rx_index的数据进行decode
    //按照解析原则，路由给各个操作函数
    //做包检查，考虑到连包的问题，最大提供5个连包

    tft_mcu_pro_data_t tft_mcu_pro_data_rec;
    uint8_t i = 0 , j=0 , k=0;
    uint16_t crc_get=0;

    if(lcd_uart_rx_index < 10)
        return ;
    
    for(i=0;i<lcd_uart_rx_index - 1 ;i++){
        //寻找包头 0xee，找到了之后做协议解析
        if(lcd_uart_rx_buf[i] == 0xee){
            //寻找到了包头，认为 i+1的位置存放了数据长度 
            tft_mcu_pro_data_rec.header = 0xee;

            tft_mcu_pro_data_rec.len = lcd_uart_rx_buf[i+1];
            tft_mcu_pro_data_rec.len <<=  8;
            tft_mcu_pro_data_rec.len |= lcd_uart_rx_buf[i+2];

            //检查1 ，是否检测到有效的帧尾

            if(lcd_uart_rx_buf[(i+tft_mcu_pro_data_rec.len+5) % LCD_UART_MAX_LEN] == 0xff &&
               lcd_uart_rx_buf[(i+tft_mcu_pro_data_rec.len+6) % LCD_UART_MAX_LEN] == 0xfd &&
               lcd_uart_rx_buf[(i+tft_mcu_pro_data_rec.len+7) % LCD_UART_MAX_LEN] == 0xfe &&
               lcd_uart_rx_buf[(i+tft_mcu_pro_data_rec.len+8) % LCD_UART_MAX_LEN] == 0xff){

                   //已经检查到了有效的帧尾
                   //检查2 ，CRC校验是否正确

                    crc_get = lcd_uart_rx_buf[(i+tft_mcu_pro_data_rec.len+3) % LCD_UART_MAX_LEN];
                    crc_get <<= 8;
                    crc_get |= lcd_uart_rx_buf[(i+tft_mcu_pro_data_rec.len+4) % LCD_UART_MAX_LEN];

                    tft_mcu_pro_data_rec.crc = lib_calculateCrc16((uint8_t *)&lcd_uart_rx_buf[(i+3) % LCD_UART_MAX_LEN],tft_mcu_pro_data_rec.len);
               
                    if(crc_get == tft_mcu_pro_data_rec.crc){

                        //CRC校验通过
                        //载入load数据，根据CMD来寻找函数

                        tft_mcu_pro_data_rec.cmd = lcd_uart_rx_buf[(i+3) % LCD_UART_MAX_LEN];
                        tft_mcu_pro_data_rec.cmd <<= 8;
                        tft_mcu_pro_data_rec.cmd |= lcd_uart_rx_buf[(i+4) % LCD_UART_MAX_LEN];

                        for(j=0;j<tft_mcu_pro_data_rec.len;j++){
                            tft_mcu_pro_data_rec.load[j] = lcd_uart_rx_buf[(i+5 + j) % LCD_UART_MAX_LEN];
                        }

                        for(k=0;k<sizeof(tft_com_func_list)/sizeof(tft_com_func_list_t);k++){
                            if(tft_com_func_list[k].cmd == tft_mcu_pro_data_rec.cmd){
                                tft_com_func_list[k].func(&tft_mcu_pro_data_rec);
                            }
                        }

                        // i+

                        i+= 5 + tft_mcu_pro_data_rec.len + 6;
                    }
               }
        }
    }

}
void arg_tft_com_handle(void){

    uint8_t tft_debug_buf[100] = "MCU_RECV:";

    if(_TFT_COM_UPDATE_FLAG == false)
        return ;

    _TFT_COM_UPDATE_FLAG = false;

    if (lcd_buf_is_ready_check() == true)
    {
        //data is in lcd_uart_rx_buf
        //len is lcd_uart_rx_index

        if(get_tft_com_transmit_sw() == true){
            if (lcd_uart_rx_index < 90)
            {
                //把hex格式文件转化为字符串
                hex_str(lcd_uart_rx_buf, lcd_uart_rx_index, tft_debug_buf + 9);
                debug_sender_str("\r\n");delay_ms(1);
                debug_sender_str(tft_debug_buf);delay_ms(10);
                debug_sender_str("\r\n");delay_ms(1);
            }
        }

        lcd_tft_cmd_decode();

        //lcd_sender(lcd_uart_rx_buf, lcd_uart_rx_index);

        //clear
        clear_lcd_uart();
    }
}

