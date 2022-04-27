#include "register_access/register_access.h"

Register GPFSEL1 = (Register)0x3F200004;
Register AUX_IRQ = (Register)0x3F215000;
Register AUX_ENABLES = (Register)0x3F215004;
Register AUX_MU_IO_REG = (Register)0x3F215040;
Register AUX_MU_IER_REG = (Register)0x3F215044;
Register AUX_MU_IIR_REG = (Register)0x3F215048;
Register AUX_MU_LCR_REG = (Register)0x3F21504C;
Register AUX_MU_MCR_REG = (Register)0x3F215050;
Register AUX_MU_LSR_REG = (Register)0x3F215054;
Register AUX_MU_CNTL_REG = (Register)0x3F215060;
Register AUX_MU_BAUD = (Register)0x3F215068;