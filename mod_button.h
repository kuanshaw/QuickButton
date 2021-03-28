/**
* @file     mod_button.h
* @brief    Button module
* @author   kuanshaw
* @date     2021-3-11
* @version  v1.0.0
*/

#ifndef __MOD_BUTTON_H
#define __MOD_BUTTON_H

#include <stdint.h>
#include <stddef.h>


// config time, ms
#define BTN_TICK_INTERVAL               10      // Unit: ms
#define BTN_DEBOUNCE_TIME               20 
#define BTN_SIGNAL_PRESS_TIME           200
#define BTN_DOUBLE_PRESS_TIME           500
#define BTN_LONG_PRESS_START_TIME       3000


typedef enum
{
    BTN_UNPRESS             = 0x00,
    BTN_EVENT_SIGNLE_CLICK  = 0x01,
    BTN_EVENT_DOUBLE_CLICK  = 0x02,
    BTN_EVENT_LONG_HOLD     = 0x04,
    _BTN_EVENT_MAX          = 4,
} button_event_t;


typedef struct
{
    uint16_t tick;
    uint16_t debounce;
    uint16_t repeat;
} btn_counter_t;


typedef struct
{
    uint8_t ispress     : 1;    ///< 1 - Button Pressed, 0 - No Press
    uint8_t setlong     : 1;
    uint8_t step        : 4;    ///< state step, max 16
    uint8_t remain      : 2;
} btn_state_t;


typedef struct button_obj_struct button_obj_t;

struct button_obj_struct
{
    button_obj_t    *next;  ///< next object
    
    btn_state_t     state;
    btn_counter_t   cnt;
    uint16_t        longpress_tick;
    uint8_t (*read_pin)(void);
    void (*btn_cb)(uint8_t event, void *arg);
    void *arg;
};


int8_t button_create(button_obj_t *obj, uint8_t (*read_pin)(void), void (*cb)(uint8_t event, void *arg), void *arg);

int8_t button_set_longpress_time(button_obj_t *obj, uint16_t time);

void button_tick(uint16_t period_ms);


#endif 

