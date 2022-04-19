#include <stdint.h>

#define GPFSEL4 0x3F200010
#define GPSET1 0x3F200020
#define GPCLR1 0x3F20002C

extern void do_nothing();

void kernel_main() {
  *(volatile uint32_t *)GPFSEL4 &= ~(7 << 21);
  *(volatile uint32_t *)GPFSEL4 |= 1 << 21;

  while (1) {
    *(volatile uint32_t *)GPSET1 = 1 << (47 - 32);

    for (uint32_t i = 0; i < 0x100000; i++) {
      do_nothing();
    }

    *(volatile uint32_t *)GPCLR1 = 1 << (47 - 32);

    for (uint32_t i = 0; i < 0x010000; i++) {
      do_nothing();
    }
  }
}