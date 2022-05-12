#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include "../gpio/gpio.h"

void wait_cycles(uint32_t cycles);
void wait_ms(uint32_t milliseconds);
unsigned char * uint_to_ascii_hex(uint32_t value);

extern void do_nothing();
extern void halt();

void set_activity_led();
void clear_activity_led();

#endif