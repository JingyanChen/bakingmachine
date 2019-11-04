#include "periph_fan.h"
#include <stdbool.h>
#include <stdint.h>
#include "csp_gpio.h"
static bool fan_status[FAN_NUM];

void periph_fan_init(void){
    uint8_t i=0;

    for(i=0;i<FAN_NUM;i++){
        fan_status[i] = false;
        fan_control(i,false);
    }
}

bool get_fan_status(uint8_t fan_id){
    return fan_status[fan_id % FAN_NUM];
}

void set_fan(uint8_t id , bool sw){
    fan_control(id % FAN_NUM ,sw);
    fan_status[id % FAN_NUM] = sw;
}

void periph_fan_handle(void){

}
