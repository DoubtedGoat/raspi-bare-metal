#include <stdint.h>
#include "register_access/register_access.h"
#include "debug/debug.h"
#include "uart.h"

Register GPPUD = (Register)0x3F200094;
Register GPPUDCLK = (Register)0x3F200098;
Register GPFSEL4 = (Register)0x3F200010;
Register GPSET1 = (Register)0x3F200020;
Register GPCLR1 = (Register)0x3F20002C;


#define SYSCLK_RATE 250000000
#define BAUDRATE 115200

const uint32_t baud_divisor = (SYSCLK_RATE / (8 * BAUDRATE)) - 1;

void set_activity_led() {
  register_bit_write(GPSET1, 15, 1);
}

void clear_activity_led() {
  register_bit_write(GPCLR1, 15, 1);
}

void print_hex(uint32_t value) {
  uint8_t * hex;
  hex = uint_to_ascii_hex(value);
  for (int i = 0; i < 8; i++) {
    while(!( register_bit_read(AUX_MU_LSR_REG, 5))) { do_nothing(); }
    register_write(AUX_MU_IO_REG, hex[i]);
  }
  while(!( register_bit_read(AUX_MU_LSR_REG, 5))) { do_nothing(); }
  register_write(AUX_MU_IO_REG, 13);
  while(!( register_bit_read(AUX_MU_LSR_REG, 5))) { do_nothing(); }
  register_write(AUX_MU_IO_REG, 10);
}

void kernel_main() {
  int read_buffer_idx = 0;
  int write_idx = 0;
  uint32_t read_buffer[32];
  uint32_t scratch_var;
  Register scratch_reg = (Register)&scratch_var;

  wait(1048576);
  register_bitfield_write(GPFSEL4, 23, 21, 001);

  // Configure GPIO 14+15 to be rx/tx for UART1
  register_bitfield_write(GPFSEL1, 17, 15, 2);
  register_bitfield_write(GPFSEL1, 14, 12, 2);
  
  // Disable pullups on UART pins.
  // This is a bit of a process - we set the config value,
  // wait 150 sysclk cycles, and then "clock" it into 
  // a specific GPIO pin
  register_write(GPPUD, 0);
  wait(150);
  register_bitfield_write(GPPUDCLK, 15, 14, 3);
  wait(150);
  register_bitfield_write(GPPUDCLK, 15, 14, 0);


  set_activity_led();
  wait(1048576);
  clear_activity_led();

  // Enable the UART
  register_write(AUX_ENABLES, 1);

  // Disable interrupts, use polling
  register_bitfield_write(AUX_MU_IER_REG, 1, 0, 0);

  // Disable TX/RX while we config
  register_write(AUX_MU_CNTL_REG, 0);

  // Set 8bit mode
  register_write(AUX_MU_LCR_REG, 3);
  //register_bitfield_write(AUX_MU_LCR_REG, 1, 0, 3);

  // Clear DLAB
  register_bit_write(AUX_MU_LCR_REG, 7, 0);

  // Clear FIFOs
  register_bitfield_write(AUX_MU_IIR_REG, 2, 1, 3);

  // Turn off transmit flow control
  register_bit_write(AUX_MU_CNTL_REG, 3, 0);
  register_write(AUX_MU_BAUD, baud_divisor);

  // Set TX and RX Enables to True
  register_write(AUX_MU_CNTL_REG, 3);
  //register_bitfield_write(AUX_MU_CNTL_REG, 2, 0, 3);

  register_write(scratch_reg, 0xDEADBEEF);
  register_bitfield_write(scratch_reg, 7, 4, 9);
  print_hex(register_read(scratch_reg));
  
  while (1) {
    read_buffer_idx = 0;
    write_idx = 0;

    // Slurp up RX FIFO
    while(register_bit_read(AUX_MU_LSR_REG, 1) &&
          (read_buffer_idx < 32)) {
        uint32_t read_value = register_read(AUX_MU_IO_REG);
        read_buffer[read_buffer_idx] = read_value;
        read_buffer_idx += 1;
    }

    // Dump our read values to TX
    while(write_idx < read_buffer_idx) {
      // Wait for spots in TX FIFO
      while(!( register_bit_read(AUX_MU_LSR_REG, 5))) { do_nothing(); }
      register_write(AUX_MU_IO_REG, read_buffer[write_idx]);
      write_idx++;
    }
  }
  
}