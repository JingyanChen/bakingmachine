#include "sys.h"
#include "csp_uart.h"
#include <string.h>
#include <stdbool.h>
#include "csp_timer.h"
#include "csp_gpio.h"
#include "arg_debug_pro.h"

uint8_t debug_uart_rx_buf[DEBUG_UART_MAX_LEN];
uint16_t debug_uart_rec_len = 0;
uint8_t pt100_uart_rx_buf[PT100_UART_MAX_LEN];
uint16_t pt100_uart_rx_index = 0;
uint8_t lcd_uart_rx_buf[LCD_UART_MAX_LEN];
uint16_t lcd_uart_rx_index = 0;

static uint8_t debug_uart_tx_buf[DEBUG_UART_MAX_LEN];
static uint16_t debug_uart_tx_len = 0;
static uint16_t debug_uart_tx_tick = 0;

static uint8_t pt100_uart_tx_buf[PT100_UART_MAX_LEN];
static uint16_t pt100_uart_rx_old = 0;
static uint16_t pt100_uart_rx_index_no_change_tick = 0;
static uint16_t pt100_uart_tx_len = 0;
static uint16_t pt100_uart_tx_tick = 0;

static uint8_t lcd_uart_tx_buf[LCD_UART_MAX_LEN];
static uint16_t lcd_uart_rx_old = 0;
static uint16_t lcd_uart_rx_index_no_change_tick = 0;
static uint16_t lcd_uart_tx_len = 0;
static uint16_t lcd_uart_tx_tick = 0;

void csp_uart_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    // IO CONFIG

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PA10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //NVIC

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // BASIC CONFIG

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);

    USART_Cmd(USART1, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART2, &USART_InitStructure);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART2, USART_IT_TXE, DISABLE);

    USART_Cmd(USART2, ENABLE);

    //usart 3

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART3, &USART_InitStructure);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART3, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //rs485 EN 0 rx 1 tx  default is rx
    rs485_enbale_control(false);

    //debug welcome ui
    debug_sender_str("welcome to comegene instruction system\r\n");
    debug_sender_str("input '?' to get help \r\n");
}

//DEBUG UART API

void debug_sender(uint8_t *sender, uint16_t len)
{

    if (len > DEBUG_UART_MAX_LEN)
        return;

    memcpy(debug_uart_tx_buf, sender, len);
    debug_uart_tx_len = len;
    debug_uart_tx_tick = 0;

    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}
void debug_sender_str(uint8_t *str)
{
    debug_sender(str, strlen((char *)str));
}
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {

        debug_uart_rx_buf[debug_uart_rec_len % DEBUG_UART_MAX_LEN] = USART_ReceiveData(USART1);
        if (debug_uart_rx_buf[debug_uart_rec_len % DEBUG_UART_MAX_LEN] == 0x8)
        {
            if (debug_uart_rec_len >= 2)
            {
                debug_uart_rx_buf[debug_uart_rec_len % DEBUG_UART_MAX_LEN] = 0;
                debug_uart_rx_buf[debug_uart_rec_len % DEBUG_UART_MAX_LEN - 1] = 0;
                debug_uart_rec_len--; //退格按键
            }
            else
            {
                debug_uart_rec_len = 0;
            }
        }
        else
        {
            debug_uart_rec_len++;
        }
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }

    if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {

        USART_SendData(USART1, debug_uart_tx_buf[debug_uart_tx_tick % DEBUG_UART_MAX_LEN]);
        debug_uart_tx_tick++;

        if (debug_uart_tx_tick > debug_uart_tx_len - 1)
        {
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
            debug_uart_tx_tick = 0;
        }
    }
}

bool debug_buf_is_ready_check(void)
{
    if (debug_uart_rec_len == 0)
        return false;

    if (debug_uart_rx_buf[debug_uart_rec_len - 2] == 0x0D && debug_uart_rx_buf[debug_uart_rec_len - 1] == 0x0A)
    {

        return true;
    }
    else
    {
        return false;
    }
}
void clear_debug_uart(void)
{
    memset(debug_uart_rx_buf, 0, debug_uart_rec_len);
    debug_uart_rec_len = 0;
}
void debug_uart_self_test_handle(void)
{
    if (debug_buf_is_ready_check())
    {
        //data is in debug_uart_rx_buf
        //len is debug_uart_rec_len - 1
        debug_sender(debug_uart_rx_buf, debug_uart_rec_len);

        //after used we must clear rx
        clear_debug_uart();
    }
}

//PT100 UART API
void pt100_sender(uint8_t *sender, uint16_t len)
{

    if (len > PT100_UART_MAX_LEN)
        return;

    memcpy(pt100_uart_tx_buf, sender, len);
    pt100_uart_tx_len = len;
    pt100_uart_tx_tick = 0;

    rs485_enbale_control(true); // set rs485 as tx

    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        pt100_uart_rx_buf[pt100_uart_rx_index % PT100_UART_MAX_LEN] = USART_ReceiveData(USART2);
        pt100_uart_rx_index++;
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }

    if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
    {
        USART_SendData(USART2, pt100_uart_tx_buf[pt100_uart_tx_tick % PT100_UART_MAX_LEN]);
        pt100_uart_tx_tick++;

        if (pt100_uart_tx_tick > pt100_uart_tx_len - 1)
        {

            USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
            pt100_uart_tx_tick = 0;

            rs485_enbale_control(false); // set rs485 as rx
        }
    }
}

bool pt100_buf_is_ready_check(void)
{

    if (pt100_uart_rx_index == 0)
        return false;

    if (_UART_PT100_10MS_FLAG == false)
        return false;

    _UART_PT100_10MS_FLAG = false;

    if (pt100_uart_rx_index != pt100_uart_rx_old)
    {
        pt100_uart_rx_index_no_change_tick = 0;
        pt100_uart_rx_old = pt100_uart_rx_index;
        return false;
    }
    else
    {
        pt100_uart_rx_index_no_change_tick++;
    }

    if (pt100_uart_rx_index_no_change_tick > 20)
    {
        pt100_uart_rx_index_no_change_tick = 0;
        return true;
    }

    return false;
}

void clear_pt100_uart(void)
{
    memset(pt100_uart_rx_buf, 0, pt100_uart_rx_index);
    pt100_uart_rx_index = 0;
    pt100_uart_rx_index_no_change_tick = 0;
}
void pt100_uart_self_test_handle(void)
{
    if (pt100_buf_is_ready_check() == true)
    {
        //data is in pt100_uart_rx_buf
        //len is pt100_uart_rx_index
        pt100_sender(pt100_uart_rx_buf, pt100_uart_rx_index);

        //clear
        clear_pt100_uart();
    }
}

void hex_str(unsigned char *inchar, unsigned int len, unsigned char *outtxt)
{
    unsigned char hbit, lbit;
    unsigned int i;

    for (i = 0; i < len; i++)
    {
        hbit = (*(inchar + i) & 0xf0) >> 4;
        lbit = *(inchar + i) & 0x0f;
        if (hbit > 9)
            outtxt[2 * i] = 'A' + hbit - 10;
        else
            outtxt[2 * i] = '0' + hbit;
        if (lbit > 9)
            outtxt[2 * i + 1] = 'A' + lbit - 10;
        else
            outtxt[2 * i + 1] = '0' + lbit;
    }

    outtxt[2 * i] = 0;
}

//lcd UART API
void lcd_sender(uint8_t *sender, uint16_t len)
{
    uint8_t tft_debug_buf[100] = "MCU_RES:";

    if (len > LCD_UART_MAX_LEN)
        return;

    memcpy(lcd_uart_tx_buf, sender, len);
    lcd_uart_tx_len = len;
    lcd_uart_tx_tick = 0;

    USART_ITConfig(USART3, USART_IT_TXE, ENABLE);

    /*
     * 如果get_tft_com_transmit_sw()返回true，认为打开了监控
     * 此时所有lcd_sender任务，将会备份一个数据发送到DEBUG口
     * 并且注明 MCU_RES:
     */
    if (get_tft_com_transmit_sw())
    {
        if (len < 90)
        {
            //把hex格式文件转化为字符串
            hex_str(sender, len, tft_debug_buf + 8);
            debug_sender_str(tft_debug_buf);
        }
    }
}

void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        lcd_uart_rx_buf[lcd_uart_rx_index % LCD_UART_MAX_LEN] = USART_ReceiveData(USART3);
        lcd_uart_rx_index++;
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }

    if (USART_GetITStatus(USART3, USART_IT_TXE) != RESET)
    {

        USART_SendData(USART3, lcd_uart_tx_buf[lcd_uart_tx_tick % LCD_UART_MAX_LEN]);
        lcd_uart_tx_tick++;

        if (lcd_uart_tx_tick > lcd_uart_tx_len - 1)
        {
            USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
            lcd_uart_tx_tick = 0;
        }
    }
}
bool lcd_buf_is_ready_check(void)
{

    if (lcd_uart_rx_index == 0)
        return false;

    if (_UART_LCD_10MS_FLAG == false)
        return false;

    _UART_LCD_10MS_FLAG = false;

    if (lcd_uart_rx_index != lcd_uart_rx_old)
    {
        lcd_uart_rx_index_no_change_tick = 0;
        lcd_uart_rx_old = lcd_uart_rx_index;
        return false;
    }
    else
    {
        lcd_uart_rx_index_no_change_tick++;
    }

    if (lcd_uart_rx_index_no_change_tick > 20)
    {
        lcd_uart_rx_index_no_change_tick = 0;
        return true;
    }
    return false;
}
void clear_lcd_uart(void)
{
    memset(lcd_uart_rx_buf, 0, lcd_uart_rx_index);
    lcd_uart_rx_index = 0;
    lcd_uart_rx_index_no_change_tick = 0;
}
void lcd_uart_self_test_handle(void)
{
    if (lcd_buf_is_ready_check() == true)
    {
        //data is in lcd_uart_rx_buf
        //len is lcd_uart_rx_index
        lcd_sender(lcd_uart_rx_buf, lcd_uart_rx_index);

        //clear
        clear_lcd_uart();
    }
}
void csp_uart_handle(void)
{
    //debug_uart_self_test_handle();
    pt100_uart_self_test_handle();
    //lcd_uart_self_test_handle();
}
