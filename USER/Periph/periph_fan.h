#ifndef __FAN__
#define __FAN__
#include <stdbool.h>
#include <stdint.h>
#define FAN_NUM 5


void periph_fan_init(void);
void periph_fan_handle(void);


bool get_fan_status(uint8_t fan_id);
void set_fan(uint8_t id , bool sw);


#endif
