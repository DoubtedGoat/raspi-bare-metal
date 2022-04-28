#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include "../register_access/register_access.h"
#include "../gpio/gpio.h"
#include "../debug/debug.h"

static const Register AUX_IRQ = (Register)0x3F215000;
static const Register AUX_ENABLES = (Register)0x3F215004;
static const Register AUX_MU_IO_REG = (Register)0x3F215040;
static const Register AUX_MU_IER_REG = (Register)0x3F215044;
static const Register AUX_MU_IIR_REG = (Register)0x3F215048;
static const Register AUX_MU_LCR_REG = (Register)0x3F21504C;
static const Register AUX_MU_MCR_REG = (Register)0x3F215050;
static const Register AUX_MU_LSR_REG = (Register)0x3F215054;
static const Register AUX_MU_CNTL_REG = (Register)0x3F215060;
static const Register AUX_MU_BAUD = (Register)0x3F215068;


enum UartConfigurationBitLength {
    UART_7_BIT = 3,
    UART_8_BIT = 0
};

enum UartConfigurationRxPins {
    UART_RX_PIN_15 = 15,
    UART_RX_PIN_33 = 33,
    UART_RX_PIN_41 = 41
};

enum UartConfigurationTxPins {
    UART_TX_PIN_14 = 14,
    UART_TX_PIN_32 = 32,
    UART_TX_PIN_40 = 40
};

typedef struct UartConfiguration {
    uint32_t baudrate;
    enum UartConfigurationBitLength bit_length;
    enum UartConfigurationRxPins rx_pin;
    enum UartConfigurationTxPins tx_pin;
} UartConfiguration;

typedef struct Uart {
    uint32_t configured;
} Uart;

int uart_configure(UartConfiguration config, Uart * uart);
int uart_write_byte(Uart * uart, uint8_t value);
int uart_read_byte(Uart * uart, uint8_t * byte);

bool uart_can_read(Uart * uart);
bool uart_can_write(Uart * uart);

#endif