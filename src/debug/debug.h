#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

void wait(uint32_t cycles);
unsigned char * uint_to_ascii_hex(uint32_t value);

extern void do_nothing();
extern void halt();

#endif