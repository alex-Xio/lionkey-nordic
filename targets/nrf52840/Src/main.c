#include "main.h"

#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_ppi.h"
#include "nrf_timer.h"

const NRF_TIMER_Type* nrf_timer = NRF_TIMER0;
const nrf_ppi_channel_t nrf_ppi_channel = NRF_PPI_CHANNEL0;

void timer_clearstart() {
    nrf_timer_task_trigger(nrf_timer, NRF_TIMER_TASK_CLEAR);
    nrf_timer_task_trigger(nrf_timer, NRF_TIMER_TASK_START);
}

void disable_gpiote_ppi() {
    nrf_gpiote_task_disable(NRF_GPIOTE, 0);
    nrf_ppi_channel_disable(NRF_PPI, nrf_ppi_channel);
}
void enable_gpiote_ppi() {
    nrf_gpiote_task_enable(NRF_GPIOTE, 0);
    nrf_ppi_channel_enable(NRF_PPI, nrf_ppi_channel);
}
void timer_led_init() {
    const nrf_timer_event_t nrf_timer_event = NRF_TIMER_EVENT_COMPARE0;

    nrf_gpio_cfg_output(LED_PIN);
    nrf_gpiote_task_configure(NRF_GPIOTE, 0, LED_PIN,
                              NRF_GPIOTE_POLARITY_TOGGLE, 0);
    nrf_gpiote_task_enable(NRF_GPIOTE, 0);

    nrf_timer_mode_set(nrf_timer, NRF_TIMER_MODE_TIMER);
    nrf_timer_bit_width_set(nrf_timer, NRF_TIMER_BIT_WIDTH_32);
    nrf_timer_prescaler_set(nrf_timer, 0);
    uint32_t ticks_500ms = nrf_timer_ms_to_ticks(500, NRF_TIMER_FREQ_16MHz);
    nrf_timer_cc_set(nrf_timer, NRF_TIMER_CC_CHANNEL0, ticks_500ms);
    nrf_timer_shorts_enable(nrf_timer, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK);

    uint32_t eep = nrf_timer_event_address_get(nrf_timer, nrf_timer_event);
    uint32_t tep =
        nrf_gpiote_task_address_get(NRF_GPIOTE, NRF_GPIOTE_TASK_OUT_0);
    nrf_ppi_channel_endpoint_setup(NRF_PPI, nrf_ppi_channel, eep, tep);
    nrf_ppi_channel_enable(NRF_PPI, nrf_ppi_channel);
    nrf_timer_task_trigger(nrf_timer, NRF_TIMER_TASK_CLEAR);
}
int main(void) {
    timer_led_init();
    for (;;) {
    }
}

void Status_LED_Set_Mode(Status_LED_Mode mode) {
    switch (mode) {
        case STATUS_LED_MODE_OFF:
            nrf_timer_task_trigger(nrf_timer, NRF_TIMER_TASK_STOP);
            disable_gpiote_ppi();
            nrf_gpio_pin_set(LED_PIN);
            break;
        case STATUS_LED_MODE_ON:
            nrf_timer_task_trigger(nrf_timer, NRF_TIMER_TASK_STOP);
            disable_gpiote_ppi();
            nrf_gpio_pin_clear(LED_PIN);
            break;
        case STATUS_LED_MODE_BLINKING_NORMAL:
            nrf_timer_task_trigger(nrf_timer, NRF_TIMER_TASK_STOP);
            uint32_t ticks_500ms =
                nrf_timer_ms_to_ticks(500, NRF_TIMER_FREQ_16MHz);
            nrf_timer_cc_set(nrf_timer, NRF_TIMER_CC_CHANNEL0, ticks_500ms);
            enable_gpiote_ppi();
            timer_clearstart();
            break;
        case STATUS_LED_MODE_BLINKING_SPECIAL:
            nrf_timer_task_trigger(nrf_timer, NRF_TIMER_TASK_STOP);
            uint32_t ticks_25ms =
                nrf_timer_ms_to_ticks(25, NRF_TIMER_FREQ_16MHz);
            nrf_timer_cc_set(nrf_timer, NRF_TIMER_CC_CHANNEL0, ticks_25ms);
            enable_gpiote_ppi();
            timer_clearstart();
            break;
    }
}
