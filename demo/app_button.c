
#include "app_button.h"

#define DBG_TAG "app/btn"
#define DBG_LVL DBG_LOG 
#include <rtdbg.h>


static uint8_t read_pin(void)
{
#if BUTTON_ACTIVE_STATE
    if(rt_pin_read(BUTTON_USER_PIN) > 0)
        return 1;
#else
    if(rt_pin_read(BUTTON_USER_PIN) == 0)
        return 1;
#endif 
    return 0;
}

static void btn_callback(uint8_t event, void *arg)
{
    switch(event)
    {
        case BTN_EVENT_SIGNLE_CLICK: 
            LOG_D("Button short click");
            break;
        
        case BTN_EVENT_DOUBLE_CLICK:
            LOG_D("Button double click");
            break;
        
        case BTN_EVENT_LONG_HOLD:
            LOG_D("Button long click");
            break;
        
        default:
            break;
    }
}


button_obj_t btn;

void button_init(void)
{
    rt_pin_mode(BUTTON_USER_PIN, PIN_MODE_INPUT_PULLUP);
    
    button_create(&btn, read_pin, btn_callback, NULL);
    
    button_set_longpress_time(&btn, 2000);
}



void button_process_10ms(void)
{
    button_tick(BTN_TICK_INTERVAL);

}





