#include "sys.h"
#include "csp_gpio.h"
#include <stdbool.h>

GPIO_TypeDef *OUTPUT_PORT_LIST[] = {
    WATER_COOL_PUMP_PORT_0,
    WATER_COOL_PUMP_PORT_1,
    WATER_COOL_PUMP_PORT_2,
    WATER_COOL_PUMP_PORT_3,
    WATER_COOL_PUMP_PORT_4,

    WATER_COOL_VAVLE_PORT_0,
    WATER_COOL_VAVLE_PORT_1,
    WATER_COOL_VAVLE_PORT_2,
    WATER_COOL_VAVLE_PORT_3,
    WATER_COOL_VAVLE_PORT_4,
    WATER_COOL_VAVLE_PORT_5,
    WATER_COOL_VAVLE_PORT_6,

    HUMIDITY_CONTROL_PORT_0,
    HUMIDITY_CONTROL_PORT_1,

    FAN_SYSTEM_CONTROL_PORT_0,
    FAN_SYSTEM_CONTROL_PORT_1,
    FAN_SYSTEM_CONTROL_PORT_2,
    FAN_SYSTEM_CONTROL_PORT_3,
    FAN_SYSTEM_CONTROL_PORT_4,

    RS485_EN_PORT,

    LED_PORT,

    MOTOR_DIR_PORT_0,
    MOTOR_DIR_PORT_1,
    MOTOR_DIR_PORT_2,
    MOTOR_DIR_PORT_3,
    MOTOR_DIR_PORT_4,

    LCD_POWER_PORT,

    MOTOR_ENABLE_PORT,

};

uint16_t OUTPUT_PIN_LIST[] = {
    WATER_COOL_PUMP_PIN_0,
    WATER_COOL_PUMP_PIN_1,
    WATER_COOL_PUMP_PIN_2,
    WATER_COOL_PUMP_PIN_3,
    WATER_COOL_PUMP_PIN_4,

    WATER_COOL_VAVLE_PIN_0,
    WATER_COOL_VAVLE_PIN_1,
    WATER_COOL_VAVLE_PIN_2,
    WATER_COOL_VAVLE_PIN_3,
    WATER_COOL_VAVLE_PIN_4,
    WATER_COOL_VAVLE_PIN_5,
    WATER_COOL_VAVLE_PIN_6,

    HUMIDITY_CONTROL_PIN_0,
    HUMIDITY_CONTROL_PIN_1,

    FAN_SYSTEM_CONTROL_PIN_0,
    FAN_SYSTEM_CONTROL_PIN_1,
    FAN_SYSTEM_CONTROL_PIN_2,
    FAN_SYSTEM_CONTROL_PIN_3,
    FAN_SYSTEM_CONTROL_PIN_4,

    RS485_EN_PIN,

    LED_PIN,

    MOTOR_DIR_PIN_0,
    MOTOR_DIR_PIN_1,
    MOTOR_DIR_PIN_2,
    MOTOR_DIR_PIN_3,
    MOTOR_DIR_PIN_4,

    LCD_POWER_PIN,

    MOTOR_ENABLE_PIN,
};

GPIO_TypeDef *INPUT_PORT_LIST[] = {
    WATER_COOL_LIQUID_FEEDBACK_PORT_0,
    WATER_COOL_LIQUID_FEEDBACK_PORT_1,
    WATER_COOL_LIQUID_FEEDBACK_PORT_2,
    WATER_COOL_LIQUID_FEEDBACK_PORT_3,
    WATER_COOL_LIQUID_FEEDBACK_PORT_4,

    MOTOR_LIMIT_PORT_0,
    MOTOR_LIMIT_PORT_1,
    MOTOR_LIMIT_PORT_2,
    MOTOR_LIMIT_PORT_3,
    MOTOR_LIMIT_PORT_4,
    MOTOR_LIMIT_PORT_5,
    MOTOR_LIMIT_PORT_6,
    MOTOR_LIMIT_PORT_7,
    MOTOR_LIMIT_PORT_8,
    MOTOR_LIMIT_PORT_9,

    KEY_IN_PORT_0,
    KEY_IN_PORT_1,
    KEY_IN_PORT_2,
    KEY_IN_PORT_3,
    KEY_IN_PORT_4,
    KEY_POWER_PORT,

};

uint16_t INPUT_PIN_LIST[] = {
    WATER_COOL_LIQUID_FEEDBACK_PIN_0,
    WATER_COOL_LIQUID_FEEDBACK_PIN_1,
    WATER_COOL_LIQUID_FEEDBACK_PIN_2,
    WATER_COOL_LIQUID_FEEDBACK_PIN_3,
    WATER_COOL_LIQUID_FEEDBACK_PIN_4,

    MOTOR_LIMIT_PIN_0,
    MOTOR_LIMIT_PIN_1,
    MOTOR_LIMIT_PIN_2,
    MOTOR_LIMIT_PIN_3,
    MOTOR_LIMIT_PIN_4,
    MOTOR_LIMIT_PIN_5,
    MOTOR_LIMIT_PIN_6,
    MOTOR_LIMIT_PIN_7,
    MOTOR_LIMIT_PIN_8,
    MOTOR_LIMIT_PIN_9,

    KEY_IN_PIN_0,
    KEY_IN_PIN_1,
    KEY_IN_PIN_2,
    KEY_IN_PIN_3,
    KEY_IN_PIN_4,
    KEY_POWER_PIN,
};

void csp_gpio_init(void){

    GPIO_InitTypeDef GPIO_InitStructure;
    uint8_t i = 0;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG, ENABLE);

    //output set

    for (i = 0; i < 28; i++)
    {

        GPIO_InitStructure.GPIO_Pin = OUTPUT_PIN_LIST[i];
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(OUTPUT_PORT_LIST[i], &GPIO_InitStructure);
        GPIO_ResetBits(OUTPUT_PORT_LIST[i], OUTPUT_PIN_LIST[i]);
    }

    //input set

    for (i = 0; i < 21; i++)
    {   
        
        GPIO_InitStructure.GPIO_Pin = INPUT_PIN_LIST[i];

        if(i>14 && i< 20){
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;//经过测试，按键需要下拉输入才可以正确工作
        }else if( i< 5 ){
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        }else{
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//限位引脚和其他按键输入，默认采用上拉输入
        }
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(INPUT_PORT_LIST[i], &GPIO_InitStructure);
    }

    //pump defualt v = 1

    for(i=0;i<5;i++){
        water_cool_pump_control(i,true);
        fan_control(i,true);
    }

    for(i=0;i<7;i++){
        water_cool_vavle_control(i,true);
    }

    humidity_control(0,true);
    humidity_control(1,true);

    //
    motor_enable_control(true);
}

void csp_gpio_handle(void){

}


//watercool system access
#define WATER_COOL_PUMP_OFFSET 0
void water_cool_pump_control(uint8_t pump_id , bool sw){
    if(sw){
        GPIO_SetBits(OUTPUT_PORT_LIST[WATER_COOL_PUMP_OFFSET + pump_id ] , OUTPUT_PIN_LIST[WATER_COOL_PUMP_OFFSET + pump_id] );
    }else{
        GPIO_ResetBits(OUTPUT_PORT_LIST[WATER_COOL_PUMP_OFFSET + pump_id ] , OUTPUT_PIN_LIST[WATER_COOL_PUMP_OFFSET + pump_id ]);
    }
}

#define WATER_COOL_VAVLE_OFFSET 5
void water_cool_vavle_control(uint8_t vavle_id , bool sw){
    if(sw){
        GPIO_SetBits(OUTPUT_PORT_LIST[WATER_COOL_VAVLE_OFFSET + vavle_id ] , OUTPUT_PIN_LIST[WATER_COOL_VAVLE_OFFSET + vavle_id ]);
    }else{
        GPIO_ResetBits(OUTPUT_PORT_LIST[WATER_COOL_VAVLE_OFFSET + vavle_id ] , OUTPUT_PIN_LIST[WATER_COOL_VAVLE_OFFSET + vavle_id ]);
    }
}

#define HUMIDITY_CONTROL_OFFSET 12
void humidity_control(uint8_t humidity_id , bool sw){
    if(sw){
        GPIO_SetBits(OUTPUT_PORT_LIST[HUMIDITY_CONTROL_OFFSET + humidity_id ] , OUTPUT_PIN_LIST[HUMIDITY_CONTROL_OFFSET+ humidity_id ]);
    }else{
        GPIO_ResetBits(OUTPUT_PORT_LIST[HUMIDITY_CONTROL_OFFSET + humidity_id ] , OUTPUT_PIN_LIST[HUMIDITY_CONTROL_OFFSET+ humidity_id ]);
    }
}

#define FAN_CONTROL_OFFSET 14
void fan_control(uint8_t id , bool sw){
    if(sw){
        GPIO_SetBits(OUTPUT_PORT_LIST[FAN_CONTROL_OFFSET + id ] , OUTPUT_PIN_LIST[FAN_CONTROL_OFFSET+ id ]);
    }else{
        GPIO_ResetBits(OUTPUT_PORT_LIST[FAN_CONTROL_OFFSET + id ] , OUTPUT_PIN_LIST[FAN_CONTROL_OFFSET+ id ]);
    }
}

void rs485_enbale_control(bool sw){
    if(sw){
        GPIO_SetBits(RS485_EN_PORT , RS485_EN_PIN);
    }else{
        GPIO_ResetBits(RS485_EN_PORT , RS485_EN_PIN);
    }    
}

void power_led_control(bool sw){
    if(sw){
        GPIO_ResetBits(LED_PORT , LED_PIN);
    }else{
        GPIO_SetBits(LED_PORT , LED_PIN);
    }      
}

void lcd_power_control(bool sw){
    if(sw){
        GPIO_ResetBits(LCD_POWER_PORT , LCD_POWER_PIN);
    }else{
        GPIO_SetBits(LCD_POWER_PORT , LCD_POWER_PIN);
    } 
}

void motor_enable_control(bool sw){
    if(sw){
        GPIO_SetBits(MOTOR_ENABLE_PORT , MOTOR_ENABLE_PIN);
    }else{
        GPIO_ResetBits(MOTOR_ENABLE_PORT , MOTOR_ENABLE_PIN);
    }     
}

#define MOTOR_DIR_CONTROL_OFFSET 21
void motor_dir_set(uint8_t id , bool sw){
    if(sw){
        GPIO_SetBits(OUTPUT_PORT_LIST[MOTOR_DIR_CONTROL_OFFSET + id ] , OUTPUT_PIN_LIST[MOTOR_DIR_CONTROL_OFFSET+ id ]);
    }else{
        GPIO_ResetBits(OUTPUT_PORT_LIST[MOTOR_DIR_CONTROL_OFFSET + id ] , OUTPUT_PIN_LIST[MOTOR_DIR_CONTROL_OFFSET+ id ]);
    }        
}

#define LIQUID_FEEDBACK_OFFSET 0
bool get_liquid_feedback_v(uint8_t id){
    return GPIO_ReadInputDataBit(INPUT_PORT_LIST[LIQUID_FEEDBACK_OFFSET + id ],INPUT_PIN_LIST[LIQUID_FEEDBACK_OFFSET + id ]);
}

#define MOTOR_LIMIT_OFFSET 5
bool get_motor_limit_v(uint8_t id){
    return GPIO_ReadInputDataBit(INPUT_PORT_LIST[MOTOR_LIMIT_OFFSET + id ],INPUT_PIN_LIST[MOTOR_LIMIT_OFFSET + id ]);
}

#define KEY_IN_OFFSET 15
bool get_key_in_v(uint8_t id){
    return GPIO_ReadInputDataBit(INPUT_PORT_LIST[KEY_IN_OFFSET + id ],INPUT_PIN_LIST[KEY_IN_OFFSET + id ]);
}

bool get_power_key_in(void){
    return GPIO_ReadInputDataBit(KEY_POWER_PORT,KEY_POWER_PIN);
}
