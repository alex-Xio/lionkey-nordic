#include "main.h"

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_ppi.h"
#include "nrf_timer.h"

void timer0_led_init() {
    nrf_ppi_channel_t nrf_ppi_channel = NRF_PPI_CHANNEL0;
    NRF_TIMER_Type* nrf_timer = NRF_TIMER0;
    nrf_timer_event_t nrf_timer_event = NRF_TIMER_EVENT_COMPARE0;

    nrf_gpio_cfg_output(6);
    nrf_gpiote_task_configure(NRF_GPIOTE, 0, 6, NRF_GPIOTE_POLARITY_TOGGLE, 0);
    nrf_gpiote_task_enable(NRF_GPIOTE, 0);

    nrf_timer_mode_set(nrf_timer, NRF_TIMER_MODE_TIMER);
    nrf_timer_bit_width_set(nrf_timer, NRF_TIMER_BIT_WIDTH_32);
    nrf_timer_prescaler_set(nrf_timer, 0);
    uint32_t time_ticks = nrf_timer_ms_to_ticks(500, NRF_TIMER_FREQ_16MHz);
    nrf_timer_cc_set(nrf_timer, NRF_TIMER_CC_CHANNEL0, time_ticks);
    nrf_timer_shorts_enable(nrf_timer, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK);

    uint32_t eep = nrf_timer_event_address_get(nrf_timer, nrf_timer_event);
    uint32_t tep =
        nrf_gpiote_task_address_get(NRF_GPIOTE, NRF_GPIOTE_TASK_OUT_0);
    nrf_ppi_channel_endpoint_setup(NRF_PPI, nrf_ppi_channel, eep, tep);
    nrf_ppi_channel_enable(NRF_PPI, nrf_ppi_channel);
    nrf_timer_task_trigger(nrf_timer, NRF_TIMER_TASK_CLEAR);
    nrf_timer_task_trigger(nrf_timer, NRF_TIMER_TASK_START);
}
int main(void) {
    timer0_led_init();
    for (;;) {
    }
}

void Status_LED_Set_Mode(Status_LED_Mode mode) {
    switch (mode) {
        case STATUS_LED_MODE_OFF:
            break;
        case STATUS_LED_MODE_ON:
            break;
        case STATUS_LED_MODE_BLINKING_NORMAL:
            break;
        case STATUS_LED_MODE_BLINKING_SPECIAL:
            break;
    }
}
