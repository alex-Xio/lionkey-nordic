#include <stdint.h>

#include "nrf.h"

extern uint32_t __isr_vector;

/**
 * I don't know why but no gcc compatible startup in nrfx/mdk contains VTOR
 * config. Without it button interrupts reset the whole MCU
 */
__attribute__((constructor(101))) static void vector_table_init(void) {
    SCB->VTOR = (uint32_t)&__isr_vector;
    __DSB();
    __ISB();
}
