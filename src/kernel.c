#include <stdint.h>
#include "register_access/register_access.h"
#include "debug/debug.h"
#include "gpio/gpio.h"
#include "uart/uart.h"
#include "interrupts/interrupts.h"

extern uint32_t *vector;

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

void kernel_main() {
  int read_buffer_idx = 0;
  int write_idx = 0;
  uint8_t read_buffer[32];
  uint32_t scratch_var;
  int return_code;
  Register scratch_reg = (Register)&scratch_var;

  wait_cycles(1000);
  gpio_pin_function_set(47, GPIO_OUTPUT);

  UartConfiguration uart_config = {
    .rx_pin = UART_RX_PIN_15,
    .tx_pin = UART_TX_PIN_14,
    .baudrate = 115200,
    .bit_length = UART_8_BIT
  };
  
  Uart uart = uart_configure(uart_config, &return_code);

  print_hex(&uart, register_read(IRQ_BASIC_PENDING));

  configure_interrupt_handler();

  while(1) {
    wait_cycles(1048576);
    print_hex(&uart, register_read(INTERRUPT_ENABLE_1));
    print_hex(&uart, register_read(IRQ_BASIC_PENDING));
    print_hex(&uart, register_read(IRQ_PENDING_1));
    print_hex(&uart, register_read(IRQ_PENDING_2));
    print_hex(&uart, register_read(AUX_MU_IIR_REG));
    print_newline(&uart);
  }

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