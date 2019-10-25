#include "arg_version.h"
#include <string.h>

void get_version_str(uint8_t * str,uint16_t len){
    uint8_t version_buf[150];
    if(IS_RELEASE == 0)
        sprintf((char*)version_buf,"V%d.%d make_time:%s_%s Debug Verion By Comegene Jingyan Chen",MAIN_VERSION,SECOND_VERSION, __DATE__, __TIME__);
    else
        sprintf((char*)version_buf,"V%d.%d make_time:%s_%s release Verion By Comegene Jingyan Chen",MAIN_VERSION,SECOND_VERSION, __DATE__, __TIME__);

    if(strlen((char*)version_buf) > len)
        return ;

    memcpy(str,version_buf,strlen((char*)version_buf));
}