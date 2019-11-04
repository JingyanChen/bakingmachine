 #ifndef __UART__
#define __UART__

#include <stdint.h>
#include <stdbool.h>
/*
 * 本文件旨在封装三路UART，
 *  DEBUG  PA9 PA10  USART1
 *  P100   PA2 PA3   USART2
    TFT    PB10 PB11 USART3
    作用如下
    DEBUG ， 用于嵌入comegene system 软件包，基于LINX配合securt-CRT使用的指令系统
    p100 , 用于转化为RS485信号，抄读PT100温度网络,基于MODBUS超时原则
    TFT ， 用于与屏幕进行通讯，基于MODEBUS超时原则

    USART1 采用指令行的结束标志0x0d作为包结尾，与另外两路做区分
 */


#define DEBUG_UART_MAX_LEN 200
#define PT100_UART_MAX_LEN 100
#define LCD_UART_MAX_LEN   200

void csp_uart_init(void);
void csp_uart_handle(void);

/*
 * DEBU_UART 采用的封装原则是以0x0d作为包的结尾
 * 下述函数给出了一个使用的范例，外部函数使用它的举例方法，访问DEBUG输入的数据
 */
void debug_uart_self_test_handle(void);

/*
 * 向DEBUG口发送报文
 */
void debug_sender(uint8_t * sender , uint16_t len);
void debug_sender_str(uint8_t * str);

//外部文件判断DEBUG数据包是否完整的方法
bool debug_buf_is_ready_check(void);
//clear uart buf
void clear_debug_uart(void);

/*
 * 利用200ms超时机制来判断包是否结束，给出了示例使用方法
 * 
 */
void pt100_uart_self_test_handle(void);

/*
 * 向PT100口发送报文
 */
void pt100_sender(uint8_t * sender , uint16_t len);

//外部文件判断PT100数据包是否完整的方法
bool pt100_buf_is_ready_check(void);
void clear_pt100_uart(void);


/*
 * 利用200ms超时机制来判断包是否结束，给出了示例使用方法
 * 
 */
void lcd_uart_self_test_handle(void);

/*
 * 向PT100口发送报文
 */
void lcd_sender(uint8_t * sender , uint16_t len);

//外部文件判断PT100数据包是否完整的方法
bool lcd_buf_is_ready_check(void);
void clear_lcd_uart(void);


//工具函数 把hex 转成 string格式
void hex_str(unsigned char *inchar, unsigned int len, unsigned char *outtxt);

//为了减少对战调用，把UART的私有变量对外公开，外部文件谨慎使用

extern uint8_t  debug_uart_rx_buf[DEBUG_UART_MAX_LEN];
extern uint16_t debug_uart_rec_len;

extern uint8_t  pt100_uart_rx_buf[PT100_UART_MAX_LEN];
extern uint16_t pt100_uart_rx_index;

extern uint8_t  lcd_uart_rx_buf[LCD_UART_MAX_LEN];
extern uint16_t lcd_uart_rx_index;

#endif
