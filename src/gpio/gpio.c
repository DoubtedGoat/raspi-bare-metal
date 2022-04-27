#include "../register_access/register_access.h"
#include "gpio.h"

Register gpio_function_registers[] = { GPFSEL0, GPFSEL1, GPFSEL2, GPFSEL3, GPFSEL4, GPFSEL5 };
void gpio_pin_function_set(uint32_t pin_number, enum GpioFunction function) {
    if ((pin_number < 0) || (pin_number > 53)) { return; } // Don't set invalid pins

    int register_number = pin_number / 10;
    int field_number = pin_number % 10;

    Register reg = gpio_function_registers[register_number];
    uint32_t low_bit = field_number * 3;
    uint32_t high_bit = low_bit + 2;
    register_bitfield_write(reg, high_bit, low_bit, function);
}

Register gpio_set_registers[] = { GPSET0, GPSET1 };
void gpio_pin_set(uint32_t pin_number) {
    int bit = pin_number % 32;
    Register reg = gpio_set_registers[pin_number / 32];

    register_bit_write(reg, bit, 1);
}

Register gpio_clear_registers[] = { GPCLR0, GPCLR1 };
void gpio_pin_clear(uint32_t pin_number) {
    int bit = pin_number % 32;
    Register reg = gpio_clear_registers[pin_number / 32];

    register_bit_write(reg, bit, 1);
}