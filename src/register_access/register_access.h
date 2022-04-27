#ifndef REGISTER_ACCESS
#define REGISTER_ACCESS

#include <stdint.h>

typedef volatile uint32_t * Register;

uint32_t register_read(Register reg);
void register_write(Register reg, uint32_t value);

uint32_t register_bitfield_read(Register reg, uint32_t bitfield_high, uint32_t bitfield_low);
void register_bitfield_write(Register reg, uint32_t bitfield_high, uint32_t bitfield_low, uint32_t value);

uint32_t register_bit_read(Register reg, uint32_t bit_position);
void register_bit_write(Register reg, uint32_t bit_position, uint32_t value);

#endif