#include "debug.h"


void wait_ms(uint32_t milliseconds) {
  uint32_t cycles = milliseconds * 250000;
  wait_cycles(cycles);
}
void wait_cycles(uint32_t cycles) {
  for (uint32_t i = 0; i < cycles; i++) {
    do_nothing();
  }
}

unsigned char hex_nibble_lookup(uint8_t value) {
  unsigned char lookup[16];
  lookup[0]= '0';
  lookup[1]= '1';
  lookup[2]= '2';
  lookup[3]= '3';
  lookup[4]= '4';
  lookup[5]= '5';
  lookup[6]= '6';
  lookup[7]= '7';
  lookup[8]= '8';
  lookup[9]= '9';
  lookup[10]= 'A';
  lookup[11]= 'B';
  lookup[12]= 'C';
  lookup[13]= 'D';
  lookup[14]= 'E';
  lookup[15]= 'F';
  return lookup[value];
}

unsigned char * uint_to_ascii_hex(uint32_t value) {
  static unsigned char hex[8];
  for (int i = 7; i >= 0; i--) { 
    hex[i] = hex_nibble_lookup(value & 15);
    value = value >> 4;
  }
  return hex;
}

void set_activity_led() {
  gpio_pin_set(47);
}

void clear_activity_led() {
  gpio_pin_clear(47);
}

