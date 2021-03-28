#ifndef __APP_BUTTON_H
#define __APP_BUTTON_H

#include "board.h"
#include "mod_button.h"

#define BUTTON_ACTIVE_STATE     0   //0 - when button be pressed, pin level is low
                                    //1 - when button be pressed, pin level is high


extern button_obj_t btn;

void button_init(void);
void button_process_10ms(void);


#endif 
