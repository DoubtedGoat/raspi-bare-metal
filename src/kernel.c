#include <stdint.h>
#include "register_access/register_access.h"
#include "debug/debug.h"
#include "gpio/gpio.h"
#include "uart.h"

#define SYSCLK_RATE 250000000
#define BAUDRATE 115200

const uint32_t baud_divisor = (SYSCLK_RATE / (8 * BAUDRATE)) - 1;

void set_activity_led() {
  gpio_pin_set(47);
}

void clear_activity_led() {
  gpio_pin_clear(47);
}

void print_newline() {
  while(!( register_bit_read(AUX_MU_LSR_REG, 5))) { do_nothing(); }
  register_write(AUX_MU_IO_REG, 13);
  while(!( register_bit_read(AUX_MU_LSR_REG, 5))) { do_nothing(); }
  register_write(AUX_MU_IO_REG, 10);
}

void print_hex(uint32_t value) {
  uint8_t * hex;
  hex = uint_to_ascii_hex(value);
  for (int i = 0; i < 8; i++) {
    while(!( register_bit_read(AUX_MU_LSR_REG, 5))) { do_nothing(); }
    register_write(AUX_MU_IO_REG, hex[i]);
  }
  print_newline();
}

void kernel_main() {
  int read_buffer_idx = 0;
  int write_idx = 0;
  uint32_t read_buffer[32];
  uint32_t scratch_var;
  Register scratch_reg = (Register)&scratch_var;

  wait(1048576);
  gpio_pin_function_set(47, GPIO_OUTPUT);


  // Configure GPIO 14+15 to be rx/tx for UART1
  gpio_pin_function_set(14, GPIO_ALT5);
  gpio_pin_function_set(15, GPIO_ALT5);

  gpio_pin_function_set(32, GPIO_ALT3);
  gpio_pin_function_set(33, GPIO_ALT3);
  gpio_pin_function_set(40, GPIO_OUTPUT);
  gpio_pin_function_set(41, GPIO_OUTPUT);
  
  // Disable pullups on UART pins.
  // This is a bit of a process - we set the config value,
  // wait 150 sysclk cycles, and then "clock" it into 
  // a specific GPIO pin
  register_write(GPPUD, 0);
  wait(200);
  register_bitfield_write(GPPUDCLK, 15, 14, 3);
  //register_write(GPPUDCLK, 3 << 14);
  wait(200);
  register_bitfield_write(GPPUDCLK, 15, 14, 0);
  //register_write(GPPUDCLK, 0);


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
    while(register_bit_read(AUX_MU_LSR_REG, 0) &&
          (read_buffer_idx < 32)) {
        uint32_t read_value = register_read(AUX_MU_IO_REG);
        read_buffer[read_buffer_idx] = read_value;
        read_buffer_idx += 1;
    }
    print_newline();
    register_write(AUX_MU_IO_REG, read_buffer_idx + 48);
    print_newline();
    // Dump our read values to TX
    while(write_idx < read_buffer_idx) {
      // Wait for spots in TX FIFO
      while(!( register_bit_read(AUX_MU_LSR_REG, 5)) ) { do_nothing(); }
      register_write(AUX_MU_IO_REG, 124);
      while(!( register_bit_read(AUX_MU_LSR_REG, 5)) ) { do_nothing(); }
      print_hex(read_buffer[write_idx]);
      //register_write(AUX_MU_IO_REG, read_buffer[write_idx]);
      write_idx++;
    }


    print_newline();
    print_newline();
    set_activity_led();
    wait(250000);
    clear_activity_led();
    wait(250000);
  }
  
}