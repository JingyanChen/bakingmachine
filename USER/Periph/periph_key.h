#ifndef __KEY__
#define __KEY__


#include <stdbool.h>
#include <stdint.h>


#define KEY_NUM 6

/*
 * 本驱动旨在提供一个对所有按键相关的驱动定义，包括按键处理函数
 * 外部文件可以通过注册事件来达到按键的效果
 * 
 * 如果外部文件没有更新按键函数指针，默认指针指向向DEBUG发送事件报文的函数
 */


void periph_key_init(void);
void periph_key_handle(void);

typedef void (*key_event_t) (void);

/*
 * brief : 外部函数注册新的函数指针给按键按下的发生函数
 * pra key_id :0-5 ,注意 0-4是推出/推入控制 5是power_key
 * pra key_event : 无返回值 无参数表的函数指针，由外部提供，如果不提供，则指向默认程序段
 * 
 */
void register_key_press_event(uint8_t key_id , key_event_t key_event);

typedef enum{
    postive=0,
    negative,
}event_type_t;

#endif
