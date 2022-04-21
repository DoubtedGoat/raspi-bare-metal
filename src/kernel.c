#include <stdint.h>
#include "uart.h"

#define GPFSEL4 0x3F200010
#define GPSET1 0x3F200020
#define GPCLR1 0x3F20002C
#define GPPUD 0x3F200094
#define GPPUDCLK 0x3F200098


#define SYSCLK_RATE 250000000
#define BAUDRATE 115200

const uint32_t baud_divisor = (SYSCLK_RATE / (8 * BAUDRATE)) - 1;

extern void do_nothing();

void set_activity_led() {
  *(volatile uint32_t *)GPSET1 = 1 << (47 - 32);
}

void clear_activity_led() {
  *(volatile uint32_t *)GPCLR1 = 1 << (47 - 32);
}

void noticeable_pause() {
  for (uint32_t i = 0; i < 0x100000; i++) {
    do_nothing();
  }
}

void wait(uint32_t cycles) {
  for (uint32_t i = 0; i < cycles; i++) {
    do_nothing();
  }
}


void kernel_main() {
  *(volatile uint32_t *)GPFSEL4 &= ~(7 << 21);
  *(volatile uint32_t *)GPFSEL4 |= 1 << 21;

  // Configure GPIO 14+15 to be rx/tx for UART1
  // Clobber all other fields #YOLO
  *(volatile uint32_t *)GPFSEL1 = (2 << 15) & (2 << 12);
  
  // Disable pullups on UART pins.
  // This is a bit of a process - we set the config value,
  // wait 150 sysclk cycles, and then "clock" it into 
  // a specific GPIO pin
  *(volatile uint32_t *)GPPUD = 0;
  wait(150);
  *(volatile uint32_t *)GPPUDCLK |= 3 << 14;
  wait(150);
  *(volatile uint32_t *)GPPUDCLK &= ~(3 << 14);

  // Enable the UART
  *(volatile uint32_t *)AUX_ENABLES = 1;

  // Disable interrupts, use polling
  *(volatile uint32_t *)AUX_MU_IER_REG &= ~(3);

  // Disable TX/RX while we config
  *(volatile uint32_t *)AUX_MU_CNTL_REG = 0;

  // Set 8bit mode
  *(volatile uint32_t *)AUX_MU_LCR_REG = 1;
  // Clear DLAB
  *(volatile uint32_t *)AUX_MU_LCR_REG &= ~(1 << 7);

  // Set RTS (seems. . . unnecessary?)
  *(volatile uint32_t *)AUX_MU_MCR_REG = 0;

  // Clear FIFOs
  *(volatile uint32_t *)AUX_MU_IIR_REG |= 3 << 1;

  // Turn off transmit flow control
  *(volatile uint32_t *)AUX_MU_CNTL_REG &= ~(1 << 3);
  *(volatile uint32_t *)AUX_MU_BAUD = baud_divisor;

  // Set TX and RX Enables to True
  *(volatile uint32_t *)AUX_MU_CNTL_REG = 3;

  while (1) {
    clear_activity_led();
    wait(0x10000);
    set_activity_led();
    wait(0x10000);
    clear_activity_led();
    noticeable_pause();
    uint32_t transmitter_empty = *(volatile uint32_t *)AUX_MU_LSR_REG & (1 << 5);
    if (transmitter_empty) {
      set_activity_led();
      *(volatile uint32_t *)AUX_MU_IO_REG = 0x5A5A5A5A;
    } else {
      clear_activity_led();
    }
    noticeable_pause();
  }
}