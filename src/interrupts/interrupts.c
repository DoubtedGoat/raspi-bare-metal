// IRQ 29 is the AUX interrupt

#include <stdbool.h>

#include "interrupts.h"
#include "../register_access/register_access.h"
#include "../gpio/gpio.h"
#include "../debug/debug.h"


extern uint32_t *vector;

bool led_on = false;

void configure_interrupt_handler() {
    // TODO: Use correct read/write barriers
    __asm__("dmb");
    //register_write(IRQ_EXCEPTION_HANDLER, *vector);

    register_bit_write(INTERRUPT_ENABLE_1, 29, 1);
    // TODO: Use correct read/write barriers
    __asm__("dmb");
}

__attribute__((interrupt("IRQ"))) void interrupt_handler() {
    // TODO: Use correct read/write barriers
    __asm__("dmb");
    if (led_on) {
        clear_activity_led();
    } else {
        set_activity_led();
    }
    // TODO: Use correct read/write barriers
    __asm__("dmb");
}