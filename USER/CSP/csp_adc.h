#ifndef __ADC__
#define __ADC__


/*
 * 本驱动旨在提供十路ADC模数转换数据
 * 使用DMA做内存搬运
 * 
 * PA 4 5 6 7 
 * PB 0 1
 * PC 0 1 2 3 
 * 
 * 用这十个作为ADC模拟口，确定其他引脚没有复用
 * 
 * PA4 ADC12 IN4
 * PA5 ADC12 IN5
 * PA6 ADC12 IN6
 * PA7 ADC12 IN7
 * PB0 ADC12 IN8
 * PB1 ADC12 IN9
 * PC0 ADC123 IN10
 * PC1 ADC123 IN11
 * PC2 ADC123 IN12
 * PC3 ADC123 IN13
 * 
 * 使用ADC1 IN4 - IN13 做一个顺序转换，使用ADC转换,DMA做数据搬运的工作。
 * 
 * 当csp_adc_int()，并且 start_adc_conver被执行
 * 会不停的更新十个通道的电压数据到最终的API中
 * 输入基准，计算出对应的电压值。
 */

#include <stdint.h>


#define ADC_INPUT_PORT_0    GPIOA
#define ADC_INPUT_PIN_0     GPIO_Pin_4
#define ADC_INPUT_PORT_1    GPIOA
#define ADC_INPUT_PIN_1     GPIO_Pin_5
#define ADC_INPUT_PORT_2    GPIOA
#define ADC_INPUT_PIN_2     GPIO_Pin_6
#define ADC_INPUT_PORT_3    GPIOA
#define ADC_INPUT_PIN_3     GPIO_Pin_7
#define ADC_INPUT_PORT_4    GPIOB
#define ADC_INPUT_PIN_4     GPIO_Pin_0
#define ADC_INPUT_PORT_5    GPIOB
#define ADC_INPUT_PIN_5     GPIO_Pin_1
#define ADC_INPUT_PORT_6    GPIOC
#define ADC_INPUT_PIN_6     GPIO_Pin_0
#define ADC_INPUT_PORT_7    GPIOC
#define ADC_INPUT_PIN_7     GPIO_Pin_1
#define ADC_INPUT_PORT_8    GPIOC
#define ADC_INPUT_PIN_8     GPIO_Pin_2
#define ADC_INPUT_PORT_9    GPIOC
#define ADC_INPUT_PIN_9     GPIO_Pin_3


void csp_adc_init(void);
void csp_adc_handle(void);

#define DEFAULT_REF_MV 3300

/*
 * brief: 获取单路电压测量的API函数，获得对应的电压值，通过输入基准换算，单位mv
 * pra @ id : 0-9 不同路的id
 * pra @ ref_mv : 基准电压，单位mv
 * return 换算后的电压值，单位mv
 */
uint16_t get_adc_v(uint8_t id ,uint16_t ref);

//外部可以直接通过访问此公开的全局变量访问各个路的电压情况
extern uint16_t adc_mv_data[10];
extern uint16_t adc_temp_data[10];

#endif
