#include "stm32f10x.h"
#include "csp_pwm.h"
#include "delay.h"
#include "sys.h"
#include "csp_timer.h"
#include "csp_gpio.h"
#include "csp_uart.h"
#include "csp_adc.h"
#include "csp_uart.h"
#include "csp_wtd.h"
#include "arg_debug_pro.h"
#include "arg_tft_com.h"
#include "arg_pid.h"
#include "periph_motor.h"
#include "periph_key.h"
#include "periph_humidity_sys.h"
#include "periph_fan.h"
#include "periph_power.h"
#include "app.h"
#include "report.h"

int main(void)
{
	delay_init();

	csp_gpio_init();
	csp_pwm_init();
	csp_timer_init();
	csp_uart_init();
	csp_adc_init();
	csp_wtd_init();

	periph_motor_init();
	periph_key_init();
	periph_humidity_sys_init();
	periph_fan_init();
	periph_power_init();
	
	arg_debug_pro_init();
	arg_tft_com_init();
	arg_pid_init();
	
	arg_app_init();
	arg_report_init();

	while (1)
	{
		csp_timer_handle();
		csp_uart_handle();
		csp_adc_handle();
		csp_pwm_handle();
		csp_wtd_handle();
		
		periph_motor_handle();
		periph_key_handle();
		periph_humidity_sys_handle();
		
		arg_debug_pro_handle();
		arg_tft_com_handle();
		arg_pid_handle();
		
		arg_app_hanlde();
		arg_report_handle();
	}
}
