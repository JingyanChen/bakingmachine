#ifndef __ARG_VERSION__
#define __ARG_VERSION__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>


#define MAIN_VERSION    0
#define SECOND_VERSION  1
#define IS_RELEASE      0


void get_version_str(uint8_t * str,uint16_t len);

#endif
