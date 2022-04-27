#include "register_access.h"

uint32_t make_bitmask(uint32_t bitfield_high, uint32_t bitfield_low) {
    return (~1 << (bitfield_low - 1)) ^ (~1 << bitfield_high);
}

uint32_t register_read(Register reg) {
    return *reg;
}
void register_write(Register reg, uint32_t value) {
    *reg = value;
}

uint32_t register_bitfield_read(Register reg, uint32_t bitfield_high, uint32_t bitfield_low) {
    uint32_t bitmask = make_bitmask(bitfield_high, bitfield_low);
    return (register_read(reg) & bitmask) >> bitfield_low;
}

// Bitfield writes are read-modify-writes so we don't step on toes
//   We mask the input as well, since we're accepting a whole word but
//   we don't want anything dumb to happen if given too many bits
void register_bitfield_write(Register reg,
    uint32_t bitfield_high,
    uint32_t bitfield_low,
    uint32_t value) {
        uint32_t bitmask = make_bitmask(bitfield_high, bitfield_low);
        uint32_t shifted_value = value << bitfield_low;
        uint32_t current_value = register_read(reg);
        
        uint32_t new_value = (current_value & ~bitmask) | (shifted_value & bitmask);
        register_write(reg, new_value);
    }

// We _could_ do a less-dumb thing, but for now we're just gonna treat
//   single-bit access as a subset of bitfield access
uint32_t register_bit_read(Register reg, uint32_t bit_position) {
    return register_bitfield_read(reg, bit_position, bit_position);
}
void register_bit_write(Register reg, uint32_t bit_position, uint32_t value) {
    return register_bitfield_write(reg, bit_position, bit_position, value);
}