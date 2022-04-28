#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "../src/register_access/register_access.h"

// TODO: All these tests are happy-path tests. We should also
//   test our guards against misuse, and strange scenarios

// Stub for ASM do_nothing
void do_nothing() {
}

// Function isn't declared in headers
uint32_t make_bitmask(uint32_t bitfield_high, uint32_t bitfield_low);
bool test_make_bitmask() {
    // Case: Normal operation
    uint32_t bitmask = make_bitmask(7, 4);
    uint32_t expected_bitmask = 0x000000F0;
    if (bitmask != expected_bitmask) {
        printf("[FAIL] Make bitmask - expected %#010x, got %#010x\n", expected_bitmask, bitmask);
        return false;
    } else {
        printf("[PASS] Make Bitmask - expected %#010x, got %#010x\n", expected_bitmask, bitmask);
    }

    bitmask = make_bitmask(0, 0);
    expected_bitmask = 0x00000001;
    if (bitmask != expected_bitmask) {
        printf("[FAIL] Make bitmask single bit - expected %#010x, got %#010x\n", expected_bitmask, bitmask);
        return false;
    } else {
        printf("[PASS] Make Bitmask single bit - expected %#010x, got %#010x\n", expected_bitmask, bitmask);
    }

    bitmask = make_bitmask(8, 8);
    expected_bitmask = 0x00000100;
    if (bitmask != expected_bitmask) {
        printf("[FAIL] Make bitmask single bit position 8 - expected %#010x, got %#010x\n", expected_bitmask, bitmask);
        return false;
    } else {
        printf("[PASS] Make Bitmask single bit position 8 - expected %#010x, got %#010x\n", expected_bitmask, bitmask);
    }

    return true;
}

bool test_bitfield_read() {
    uint32_t reg_value = 0xABCD1234;
    Register reg = (Register)&reg_value;

    uint32_t expected = 0xD1;
    uint32_t actual = register_bitfield_read(reg, 19, 12);
    if (expected != actual) {
        printf("[FAIL] Bitfield read - expected %#010x, got %#010x\n", expected, actual);
        return false;
    } else {
        printf("[PASS] Bitfield read - expected %#010x, got %#010x\n", expected, actual);
    }
    return true;
}

// TODO: This should test for "too wide" inputs, too
bool test_bitfield_write() {
    uint32_t reg_value = 0xA5A5A5A5;
    Register reg = (Register)&reg_value;

    register_bitfield_write(reg, 19, 12, 0xD1);

    uint32_t expected = 0xA5AD15A5;
    if (reg_value != expected) {
        printf("[FAIL] Bitfield write - expected %#010x, got %#010x\n", expected, reg_value);
        return false;
    } else {
        printf("[PASS] Bitfield write - expected %#010x, got %#010x\n", expected, reg_value);
    }
    return true;
}

bool test_bit_read() {
    uint32_t reg_value = 0xA5A58001;
    Register reg = (Register)&reg_value;
    uint32_t actual = register_bit_read(reg, 15);
    uint32_t expected = 1;
    if (expected != actual) {
        printf("[FAIL] Bit read position 15 - expected %#010x, got %#010x\n", expected, actual);
        return false;
    } else {
        printf("[PASS] Bit read position 15 - expected %#010x, got %#010x\n", expected, actual);
    }

    actual = register_bit_read(reg, 0);
    expected = 1;
    if (expected != actual) {
        printf("[FAIL] Bit read position 0 - expected %#010x, got %#010x\n", expected, actual);
        return false;
    } else {
        printf("[PASS] Bit read position 0 - expected %#010x, got %#010x\n", expected, actual);
    }
    return true;
}

bool test_bit_write() {
    uint32_t reg_value = 0xA5A5A5A5;
    Register reg = (Register)&reg_value;

    register_bit_write(reg, 19, 1);

    uint32_t expected = 0xA5ADA5A5;
    if (reg_value != expected) {
        printf("[FAIL] Bit write - expected %#010x, got %#010x\n", expected, reg_value);
        return false;
    } else {
        printf("[PASS] Bit write - expected %#010x, got %#010x\n", expected, reg_value);
    }
    return true;
}


int main() {
    test_make_bitmask();
    test_bitfield_read();
    test_bitfield_write();
    test_bit_read();
    test_bit_write();
}