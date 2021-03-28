/**
* @file     mod_button.c
* @brief    Button module
* @author   kuanshaw
* @date     2021-3-11
* @version  v1.0.0
*/

#include "mod_button.h"

#define BTN_DEBUG_ENABLE
#ifdef  BTN_DEBUG_ENABLE
#include <rtthread.h>
#define BTN_DEBUG(fmt, ...)  rt_kprintf("[BtnLog]: "fmt, ##__VA_ARGS__)
#else
#define BTN_DEBUG(fmt, ...)
#endif

#define _DEBOUNCE_TICKS     (BTN_DEBOUNCE_TIME         / BTN_TICK_INTERVAL)
#define _SHORT_TICKS        (BTN_SIGNAL_PRESS_TIME     / BTN_TICK_INTERVAL)
#define _DOUBLE_TICKS       (BTN_DOUBLE_PRESS_TIME     / BTN_TICK_INTERVAL)
#define _LONG_TICKS         (BTN_LONG_PRESS_START_TIME / BTN_TICK_INTERVAL)

#define BTN_STATE_STEP_START            0x00
#define BTN_STATE_STEP_PRESS_DOWN       0x01
#define BTN_STATE_STEP_PRESS_UP         0x02
#define BTN_STATE_STEP_HOLD             0x03

static button_obj_t *_head = NULL;


static void _obj_reset(button_obj_t *obj)
{
    obj->next          = NULL;
    obj->state.ispress = 0;
    obj->state.setlong = 0;
    obj->state.step    = 0;
    obj->cnt.tick      = 0;
    obj->cnt.debounce  = 0;
    obj->cnt.repeat    = 0;
    obj->longpress_tick = 0;
    obj->read_pin      = NULL;
    obj->btn_cb        = NULL;
    obj->arg           = NULL;
}


/** 
 * @brief      Create Button target
 * @param[in]  obj Button object handler
 * @param[in]  read_pin The function that read pin state, return 1 when button be pressed
 * @param[in]  cb The function that callback when some event happend
 * @param[in]  arg User arguments
 * @retval     0 Create Success, -1 Create Fail
 * @note       
 */ 
int8_t button_create(button_obj_t *obj, uint8_t (*read_pin)(void), void (*cb)(uint8_t event, void *arg), void *arg)
{
    button_obj_t *_obj = _head;
    
    if((obj == NULL) || (read_pin == NULL) || (cb == NULL))
        return -1;
    
    _obj_reset(obj);
    
    // TODO: insert the new Object in the head of list
    while(_obj)
    {
        if(_obj == obj)
            return -2;
        _obj = _obj->next;
    }
    obj->next = _head;
    _head = obj;
    
    // TODO: Updata Object data
    obj->read_pin   = read_pin;
    obj->btn_cb     = cb;
    obj->arg        = arg;

    return 0;
}

int8_t button_set_longpress_time(button_obj_t *obj, uint16_t time)
{
    if(time < 1000)
        return -1;

    obj->longpress_tick = (time / BTN_TICK_INTERVAL);
    obj->state.setlong = 1;
    
    BTN_DEBUG("Set long press time success: %d\r\n", time);
    
    return 0;
}



static void _handler(button_obj_t *obj)
{
#define BTN_EVENT_CB(event, arg)            obj->btn_cb(event, arg)
    
    uint8_t pin_state = 0;
    
    // TODO: counter
    if(obj->state.step > BTN_STATE_STEP_START)
        obj->cnt.tick++;
    
    // TODO: Read PIN state, and update obj state
    pin_state = obj->read_pin() > 0 ? 1 : 0;
    if(obj->state.ispress != pin_state)
    {
        if(++obj->cnt.debounce >= _DEBOUNCE_TICKS)
        {
            obj->state.ispress = pin_state;   // Update state 
            obj->cnt.debounce = 0;
        }
    }
    else
    {
        obj->cnt.debounce = 0;
    }
    
//    BTN_DEBUG("step: %d, tick: %d, state: %d, repeat: %d\r\n", obj->state.step, obj->cnt.tick, obj->state.ispress, obj->cnt.repeat);
    
    // TODO: state machine
    switch(obj->state.step)
    {
        case BTN_STATE_STEP_START:
            if(obj->state.ispress)    // first press
            {
                obj->state.step = BTN_STATE_STEP_PRESS_DOWN;
                obj->cnt.tick = 0;
                obj->cnt.repeat = 1;
            }
            break;
            
        case BTN_STATE_STEP_PRESS_DOWN:
            if(!obj->state.ispress)   //press up
            {
                if(obj->cnt.tick > _SHORT_TICKS)
                {
                    if(obj->cnt.repeat == 1)
                        obj->state.step = BTN_STATE_STEP_PRESS_UP;
                    else if(obj->cnt.repeat == 2)
                    {
                        obj->cnt.tick = 0;
                        BTN_EVENT_CB(BTN_EVENT_DOUBLE_CLICK, obj->arg); 
                        obj->state.step = BTN_STATE_STEP_START; //restart
                    }
                        
                    obj->cnt.tick = 0;
                }
            }
            else if(obj->state.setlong)
            {
                if(obj->cnt.tick > obj->longpress_tick)
                {
                    obj->state.step = BTN_STATE_STEP_HOLD;
                    obj->cnt.tick = 0;
                }
            }
            else if(obj->cnt.tick > _LONG_TICKS)
            {
                obj->state.step = BTN_STATE_STEP_HOLD;
                obj->cnt.tick = 0;
            }
            else
            {
            }
            break;
            
        case BTN_STATE_STEP_PRESS_UP:
            if(obj->state.ispress)    //still press on
            {
                obj->cnt.repeat = 2;
                obj->state.step = BTN_STATE_STEP_PRESS_DOWN;
                obj->cnt.tick = 0;
            }
            else if(obj->cnt.tick > _DOUBLE_TICKS)
            {
                obj->cnt.tick = 0;
                BTN_EVENT_CB(BTN_EVENT_SIGNLE_CLICK, obj->arg);    //SIGNAL
                obj->state.step = BTN_STATE_STEP_START; //restart
            }
            else
            {
            }
            break;
        
        case BTN_STATE_STEP_HOLD:
            if(obj->state.ispress)
            {
                obj->cnt.tick = 0;
                BTN_EVENT_CB(BTN_EVENT_LONG_HOLD, obj->arg);  //LONG
            }
            else
            {
                obj->state.step = BTN_STATE_STEP_START; //restart
            }
            break;
            
        default: 
            break;
    }
}


/** 
 * @brief      Button Tick Handler
 * @param[in]  period_ms The period of calling
 * @note       
 */ 
void button_tick(uint16_t period_ms)
{
    button_obj_t *obj;
    
    for(obj = _head; obj; obj = obj->next)
    {
        _handler(obj);
    }
}

