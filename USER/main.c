#include "stm32f10x.h"
#include "csp_pwm.h"
#include "delay.h"
#include "sys.h"

void close_beep (void){
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��GPIOB�˿�ʱ��
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 //BEEP-->PB.8 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //�ٶ�Ϊ50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);	 //���ݲ�����ʼ��GPIOB.8
 
 GPIO_ResetBits(GPIOB,GPIO_Pin_8);//���0���رշ��������
}

int main(void){	
    delay_init();

		close_beep();
    csp_pwm_init();
	
		set_pwm(0,0.3);
		set_pwm(1,0.3);
		set_pwm(2,0.3);
		set_pwm(3,0.3);
		set_pwm(4,0.3);
    while(1){
        
    }
}
