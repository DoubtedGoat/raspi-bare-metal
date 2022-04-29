#include <stdint.h>
#include "register_access/register_access.h"
#include "debug/debug.h"
#include "gpio/gpio.h"
#include "uart/uart.h"

void set_activity_led() {
  gpio_pin_set(47);
}

void clear_activity_led() {
  gpio_pin_clear(47);
}

void halt_on_error(int rc) { 
  if (rc >= 0) { return; }
  set_activity_led();
  halt();
}

void print_newline(Uart * uart) {
  int return_code;
  uart_write_byte(uart, 13, &return_code);
  halt_on_error(return_code);
  uart_write_byte(uart, 10, &return_code);
  halt_on_error(return_code);
}

void print_hex(Uart * uart, uint32_t value) {
  uint8_t * hex;
  int return_code;
  hex = uint_to_ascii_hex(value);
  for (int i = 0; i < 8; i++) {
    uart_write_byte(uart, hex[i], &return_code);
    halt_on_error(return_code);
  }
  print_newline(uart);
}

void print_byte(uint8_t value) {
    while(!( register_bit_read(AUX_MU_LSR_REG, 5))) { do_nothing(); }
    register_write(AUX_MU_IO_REG, value);
}

void kernel_main() {
  int read_buffer_idx = 0;
  int write_idx = 0;
  uint8_t read_buffer[32];
  uint32_t scratch_var;
  int return_code;
  Register scratch_reg = (Register)&scratch_var;

  wait(1048576);
  gpio_pin_function_set(47, GPIO_OUTPUT);

  UartConfiguration uart_config = {
    .rx_pin = UART_RX_PIN_15,
    .tx_pin = UART_TX_PIN_14,
    .baudrate = 115200,
    .bit_length = UART_8_BIT
  };
  
  Uart uart = uart_configure(uart_config, &return_code);
  halt_on_error(return_code);
  
  while (1) {
    read_buffer_idx = 0;
    write_idx = 0;

    // Slurp up RX FIFO
    while(uart_can_read(&uart, &return_code) &&
          (read_buffer_idx < 32)) {
        uint8_t byte = uart_read_byte(&uart, &return_code);
        halt_on_error(return_code);
        read_buffer[read_buffer_idx] = byte;
        read_buffer_idx += 1;
    }

    // Dump our read values to TX
    while(write_idx < read_buffer_idx) {
      // Wait for spots in TX FIFO
      uart_write_byte(&uart, read_buffer[write_idx], &return_code);
      halt_on_error(return_code);
      write_idx++;
    }
  }
  
}