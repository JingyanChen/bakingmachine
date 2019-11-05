#ifndef __WTD__
#define __WTD__

#include "sys.h"

/*
 * 由于MCU不关机的原则，所以必须提防出现异常问题的情况
 * 使用STM32内部的看门狗系统
 * 在wtd handle 中定时喂狗
 * 
 * 看门狗使用i watch dog 内部时钟40K左右
 */

void csp_wtd_init(void);
void csp_wtd_handle(void);

#endif


