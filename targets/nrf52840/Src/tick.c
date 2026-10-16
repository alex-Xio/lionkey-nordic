#include "tick.h"

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_timer.h"
uint32_t ticks = 0;
uint8_t led = 0;

void tick_init() {
    nrf_timer_mode_set(NRF_TIMER1, NRF_TIMER_MODE_TIMER);
    nrf_timer_bit_width_set(NRF_TIMER1, NRF_TIMER_BIT_WIDTH_32);
    nrf_timer_prescaler_set(NRF_TIMER1, 0);
    uint32_t ticks_1ms = nrf_timer_ms_to_ticks(1, NRF_TIMER_FREQ_16MHz);
    nrf_timer_cc_set(NRF_TIMER1, NRF_TIMER_CC_CHANNEL0, ticks_1ms);
    nrf_timer_int_enable(NRF_TIMER1, NRF_TIMER_INT_COMPARE0_MASK);
    nrf_timer_shorts_enable(NRF_TIMER1, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK);
    NVIC_EnableIRQ(TIMER1_IRQn);

    nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_CLEAR);
    nrf_timer_task_trigger(NRF_TIMER1, NRF_TIMER_TASK_START);
}

uint32_t get_ticks() { return ticks; }

void TIMER1_IRQHandler() {
    if (nrf_timer_event_check(NRF_TIMER1, NRF_TIMER_EVENT_COMPARE0)) {
        ticks++;
        nrf_timer_event_clear(NRF_TIMER1, NRF_TIMER_EVENT_COMPARE0);
    }
}
