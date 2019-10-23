#include "periph_key.h"
#include "csp_uart.h"
#include "csp_gpio.h"
#include "csp_timer.h"


static void default_key_event_0(void){debug_sender_str((uint8_t*)"key 0 event happend!\r\n");}
static void default_key_event_1(void){debug_sender_str((uint8_t*)"key 1 event happend!\r\n");}
static void default_key_event_2(void){debug_sender_str((uint8_t*)"key 2 event happend!\r\n");}
static void default_key_event_3(void){debug_sender_str((uint8_t*)"key 3 event happend!\r\n");}
static void default_key_event_4(void){debug_sender_str((uint8_t*)"key 4 event happend!\r\n");}
static void default_key_event_5(void){debug_sender_str((uint8_t*)"key 5 event happend!\r\n");}

static key_event_t key_event_func[KEY_NUM]={
    default_key_event_0,
    default_key_event_1,
    default_key_event_2,
    default_key_event_3,
    default_key_event_4,
    default_key_event_5,
};

static event_type_t key_event_v[KEY_NUM]={
    postive,
    postive,
    postive,
    postive,
    postive,
    negative,
};

static bool last_key_v[KEY_NUM]={
    false,false,false,false,false,false,
};

void periph_key_init(void){
    uint8_t i=0;

    for(i=0;i<KEY_NUM;i++){
        last_key_v[i] = get_key_in_v(i % 6);
    }
}

void periph_key_handle(void){
    uint8_t i=0;

    if(_KEY_EVENT_CHECK_FLAG == false)
        return;

    _KEY_EVENT_CHECK_FLAG = false;

    for(i=0;i<KEY_NUM;i++){
        if(key_event_v[i] == postive){

            if(last_key_v[i] == false && get_key_in_v(i % 6) == true){
                key_event_func[i]();
            }

        }else{

            if(last_key_v[i] == true && get_key_in_v(i % 6) == false){
                key_event_func[i]();
            }            
        }

        last_key_v[i] = get_key_in_v(i % 6);
    }
}

void register_key_press_event(uint8_t key_id , key_event_t key_event){
    key_event_func[key_id] = key_event;
}
