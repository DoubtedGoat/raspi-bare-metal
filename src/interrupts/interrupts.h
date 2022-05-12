#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "../register_access/register_access.h"

static const Register IRQ_BASIC_PENDING = (Register)0x3F00B200;
static const Register IRQ_PENDING_1 = (Register)0x3F00B204;
static const Register IRQ_PENDING_2 = (Register)0x3F00B208;
static const Register INTERRUPT_ENABLE_1 = (Register)0x3F00B210;
static const Register INTERRUPT_ENABLE_2 = (Register)0x3F00B214;


void configure_interrupt_handler();

#endif