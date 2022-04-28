#ifndef GPIO_H
#define GPIO_H

static const Register GPFSEL0 = (Register)0x3F200000;
static const Register GPFSEL1 = (Register)0x3F200004;
static const Register GPFSEL2 = (Register)0x3F200008;
static const Register GPFSEL3 = (Register)0x3F20000C;
static const Register GPFSEL4 = (Register)0x3F200010;
static const Register GPFSEL5 = (Register)0x3F200014;

static const Register GPSET0 = (Register)0x3F20001C;
static const Register GPSET1 = (Register)0x3F200020;

static const Register GPCLR0 = (Register)0x3F200028;
static const Register GPCLR1 = (Register)0x3F20002C;

static const Register GPPUD = (Register)0x3F200094;
static const Register GPPUDCLK = (Register)0x3F200098;

enum GpioFunction {
    GPIO_INPUT = 0,
    GPIO_OUTPUT = 1,
    GPIO_ALT0 = 4,
    GPIO_ALT1 = 5,
    GPIO_ALT2 = 6,
    GPIO_ALT3 = 7,
    GPIO_ALT4 = 3,
    GPIO_ALT5 = 2
};

void gpio_pin_function_set(uint32_t pin_number, enum GpioFunction function);
void gpio_pin_set(uint32_t pin_number);
void gpio_pin_clear(uint32_t pin_number);

#endif