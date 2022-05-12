#include "uart.h"

#define SYSCLK_RATE 250000000
#define BAUDRATE 115200

const uint32_t baud_divisor = (SYSCLK_RATE / (8 * BAUDRATE)) - 1;


void set_error(int * return_code, int value)  {
    if (return_code == 0) { return; }
    *return_code = value;
    return;
}

bool valid_uart_configuration(UartConfiguration config) {
    // We're going to set bit flags for each type of error,
    //   and then not use them
    // It'll be there if we ever decide to do checking
    uint32_t error = 0;
    switch(config.rx_pin) {
        case UART_RX_PIN_15:
        case UART_RX_PIN_33:
        case UART_RX_PIN_41:
            break;
        default: 
            error |= 1 << 0;
            break;
    }

    switch(config.tx_pin) {
        case UART_TX_PIN_14:
        case UART_TX_PIN_32:
        case UART_TX_PIN_40:
            break;
        default: 
            error |= 1 << 1;
            break;
    }

    switch(config.bit_length) {
        case UART_7_BIT:
        case UART_8_BIT:
            break;
        default:
            error |= 1 << 2;
            break;
    }

    return !error;
}

Uart uart_configure(UartConfiguration config, int * return_code) {
    // Extremely aggressive memory barrier for AXI peripheral access,
    //   until I can be bothered to do this correctly
    // TODO: Use correct read/write barriers
    __asm__("dmb");
    Uart uart = {
        .configured = false
    };
    // Validate our config
    if (!valid_uart_configuration(config)) { 
        set_error(return_code, -1);
        return uart;
    }

    // Configure GPIO 14+15 to be rx/tx for UART1
    gpio_pin_function_set(config.rx_pin, GPIO_ALT5);
    gpio_pin_function_set(config.tx_pin, GPIO_ALT5);


    // Disable pullups on UART pins.
    // This is a bit of a process - we set the config value,
    // wait 150 sysclk cycles, and then "clock" it into 
    // a specific GPIO pin
    register_write(GPPUD, 0);
    wait_cycles(200);
    register_bit_write(GPPUDCLK, config.rx_pin, 1);
    wait_cycles(200);
    register_bit_write(GPPUDCLK, config.rx_pin, 0);

    wait_cycles(200);
    register_bit_write(GPPUDCLK, config.tx_pin, 1);
    wait_cycles(200);
    register_bit_write(GPPUDCLK, config.tx_pin, 0);

    // Enable the UART
    register_bit_write(AUX_ENABLES, 0, 1);

    // Disable interrupts, use polling
    register_bitfield_write(AUX_MU_IER_REG, 1, 0, 2);

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

    uart.configured = true;
    // TODO: Use correct read/write barriers
    __asm__("dmb");
    return uart;
}

void uart_write_byte(Uart * uart, uint8_t value, int * return_code) {
    // TODO: Use correct read/write barriers
    __asm__("dmb");
    if (!(uart->configured)) { return; }
    while(!( uart_can_write(uart, 0)) ) { do_nothing(); }
    register_write(AUX_MU_IO_REG, value);
    // TODO: Use correct read/write barriers
    __asm__("dmb");
}

uint8_t uart_read_byte(Uart * uart, int * return_code) {
    // TODO: Use correct read/write barriers
    __asm__("dmb");
    if (!(uart->configured)) { 
        set_error(return_code, -1);
        return 0; 
    }
    if (!uart_can_read(uart, 0)) {
        set_error(return_code, -1);
        return 0;
    }
    uint32_t result = register_read(AUX_MU_IO_REG);
    // TODO: Use correct read/write barriers
    __asm__("dmb");
    return result;
}

bool uart_can_read(Uart * uart, int * return_code) {
    // TODO: Use correct read/write barriers
    __asm__("dmb");
    if (!(uart->configured)) {
        set_error(return_code, -1);
        return false;
    }
    uint32_t result = register_bit_read(AUX_MU_LSR_REG, 0);
    // TODO: Use correct read/write barriers
    __asm__("dmb");
    return result;
}

bool uart_can_write(Uart * uart, int * return_code) {
    // TODO: Use correct read/write barriers
    __asm__("dmb");
    if (!(uart->configured)) {
        set_error(return_code, -1);
        return false;
    }
    uint32_t result = register_bit_read(AUX_MU_LSR_REG, 5);
    // TODO: Use correct read/write barriers
    __asm__("dmb");
    return result;
}