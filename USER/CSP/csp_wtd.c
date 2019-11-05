#include "csp_wtd.h"

#define SK_IWDG_PER             64   //40khz/64=625HZ 64 Prescaler divider to 625
#define SK_IWDG_CNT             1250 // 2s timeout

void csp_wtd_init(void){

    // First Open LSI Clock for IWDG    
    RCC_LSICmd(ENABLE);    
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);     
    
    // If use IWDG, LSI will be opened force    
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);    
    // Set Prescaler    
    IWDG_SetPrescaler(SK_IWDG_PER);    
    // Set reload value    
    IWDG_SetReload(SK_IWDG_CNT);    
    // Set 0xAAAA To make sure not go into reset    
    IWDG_ReloadCounter();    
    // Enable WDG    
    IWDG_Enable();

}

//1s内必须喂狗一次，否则认为是错误情况，复位创奇
void csp_wtd_handle(void){
    IWDG_ReloadCounter();
}
