#include "main.h"

#include <stdint.h>

#include "nrf.h"

int main(void) {
    NRF_P0->DIRSET = 1u << 6;
    for (;;) {
        NRF_P0->OUTCLR = 1u << 6;  // LED on (active low)
        for (volatile uint32_t i = 0; i < 400000; i++) {
        }
        NRF_P0->OUTSET = 1u << 6;
        for (volatile uint32_t i = 0; i < 400000; i++) {
        }
    }
}