#ifndef __LED_H
#define __LED_H

#include <p33Fxxxx.h>
#include "types.h"

//do not change the order of the following 3 definitions
#include <libpic30.h>


void led_initialize(void);
void led_set(int port,int value);

#endif