#include "csp_adc.h"
#include "csp_timer.h"
#include "sys.h"
#include <string.h>

GPIO_TypeDef *ADC_PORT_LIST[] = {
    ADC_INPUT_PORT_0,
    ADC_INPUT_PORT_1,
    ADC_INPUT_PORT_2,
    ADC_INPUT_PORT_3,
    ADC_INPUT_PORT_4,
    ADC_INPUT_PORT_5,
    ADC_INPUT_PORT_6,
    ADC_INPUT_PORT_7,
    ADC_INPUT_PORT_8,
    ADC_INPUT_PORT_9,
};

uint16_t ADC_PIN_LIST[] = {
    ADC_INPUT_PIN_0,
    ADC_INPUT_PIN_1,
    ADC_INPUT_PIN_2,
    ADC_INPUT_PIN_3,
    ADC_INPUT_PIN_4,
    ADC_INPUT_PIN_5,
    ADC_INPUT_PIN_6,
    ADC_INPUT_PIN_7,
    ADC_INPUT_PIN_8,
    ADC_INPUT_PIN_9,
};
#define ADC_AVERAGE_NUM 100
#define ADC_CHANNEL_NUM 10

uint16_t adc_result[ADC_AVERAGE_NUM][ADC_CHANNEL_NUM];


void start_adc_conver(void)
{
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    DMA_Cmd(DMA1_Channel1, ENABLE); //启动DMA通道
}


void csp_adc_init(void)
{
    //使用ADC1做十路转换，转换结果直接DMA拉取到内存，不允许打扰CPU处理其他事务
    uint8_t i = 0;

    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    RCC_ADCCLKConfig(RCC_PCLK2_Div6); //ADC的输入时钟为12MHZ

    //IO CONFIG
    for (i = 0; i < ADC_CHANNEL_NUM; i++)
    {
        GPIO_InitStructure.GPIO_Pin = ADC_PIN_LIST[i];
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
        GPIO_Init(ADC_PORT_LIST[i], &GPIO_InitStructure);
    }

    ADC_DeInit(ADC1);

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                  //独立模式
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;                        //扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                  //连续转换模式
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //外部触发转换关闭
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;              //ADC数据右对齐
    ADC_InitStructure.ADC_NbrOfChannel = 10;                            //顺序扫描的通道数量
    ADC_Init(ADC1, &ADC_InitStructure);

    //配置采样顺序，采样顺序为第三个参数，采样时间239个周期
    ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 2, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 3, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 4, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 5, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 6, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 7, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 8, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 9, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 10, ADC_SampleTime_239Cycles5);

    ADC_DMACmd(ADC1, ENABLE); //打开DMA支持

    ADC_Cmd(ADC1, ENABLE); //使能ADC

    //复位后，校准ADC

    ADC_ResetCalibration(ADC1);

    while (ADC_GetResetCalibrationStatus(ADC1))
        ;

    ADC_StartCalibration(ADC1);

    while (ADC_GetCalibrationStatus(ADC1))
        ;

    //DMA相关配置

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit(DMA1_Channel1);                                                  //将DMA的通道1寄存器重设为缺省值
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR;                  //DMA外设ADC基地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&adc_result;                    //DMA内存基地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                          //内存作为数据传输的目的地
    DMA_InitStructure.DMA_BufferSize = ADC_AVERAGE_NUM * ADC_CHANNEL_NUM;       //DMA通道的DMA缓存的大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            //外设地址寄存器不变
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     //内存地址寄存器递增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //数据宽度为16位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;         //数据宽度为16位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                             //工作在循环缓存模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;                         //DMA通道 x拥有高优先级
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                //DMA通道x没有设置为内存到内存传输
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);                                //根据DMA_InitStruct中指定的参数初始化DMA的通道

    //start conver

    start_adc_conver();
}


uint16_t get_adc_v(uint8_t id ,uint16_t ref){
    
    uint16_t i=0;
    uint32_t sum=0;
    uint16_t rlt=0;

    for(i=0;i<ADC_AVERAGE_NUM;i++){
        sum += adc_result[i][id];
    }

    sum /= ADC_AVERAGE_NUM;
    
    // 
    rlt = (uint16_t)(((float)sum / 4096.0) * (float)ref) ;  

    return rlt;
}

uint16_t adc_mv_data[10];
uint16_t adc_temp_data[10];
void csp_adc_handle(void)
{
    //200ms更新一次十路电压数据，后期考虑做换算转化为温度单位
    //外部可以通过对应API访问数据
    uint8_t i=0;

    if(_UPDATE_ADC_DATA_FLAG == false)
        return ;

    _UPDATE_ADC_DATA_FLAG = false;

    for(i=0;i<ADC_CHANNEL_NUM;i++){
        adc_mv_data[i] = get_adc_v(i,DEFAULT_REF_MV);
        /*
         * 目前的计算方法，标定法,50欧姆
         * 源端108欧姆 对应389mV
         * 4mA - 0摄氏度 20mA -100摄氏度
         * 4mA - 200mv   20mA - 1000mV
         * 0摄氏度 - 200mV   100摄氏度 - 1000mV
         */
        adc_temp_data[i] = (uint16_t)((float)( (float)adc_mv_data[i] - (float)200 ) / 0.8);
    }
}

