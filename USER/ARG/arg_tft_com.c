#include "csp_uart.h"
#include "csp_timer.h"
#include "arg_debug_pro.h"
#include "delay.h"

#include <stdbool.h>
#include <string.h>

void arg_tft_com_init(void){

}


static void lcd_tft_cmd_decode(void){
    //对lcd_uart_rx_buf长度为lcd_uart_rx_index的数据进行decode
    lcd_sender(lcd_uart_rx_buf,lcd_uart_rx_index);
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

