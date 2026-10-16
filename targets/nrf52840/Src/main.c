#include "main.h"

#include "app.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_ppi.h"
#include "nrf_timer.h"

NRF_TIMER_Type* nrf_timer = NRF_TIMER0;

volatile ButtonState UserButtonState = BUTTON_RELEASED;

void timer_clearstart() {
    nrf_timer_task_trigger(nrf_timer, NRF_TIMER_TASK_CLEAR);
    nrf_timer_task_trigger(nrf_timer, NRF_TIMER_TASK_START);
}

void disable_gpiote_ppi() {
    nrf_gpiote_task_disable(NRF_GPIOTE, 0);
    nrf_ppi_channel_disable(NRF_PPI, NRF_PPI_CHANNEL0);
}
void enable_gpiote_ppi() {
    nrf_gpiote_task_enable(NRF_GPIOTE, 0);
    nrf_ppi_channel_enable(NRF_PPI, NRF_PPI_CHANNEL0);
}
void timer_led_init() {
    const nrf_timer_event_t nrf_timer_event = NRF_TIMER_EVENT_COMPARE0;

    nrf_gpio_cfg_output(LED_PIN);
    nrf_gpiote_task_configure(NRF_GPIOTE, 0, LED_PIN,
                              NRF_GPIOTE_POLARITY_TOGGLE, 1);
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
    nrf_ppi_channel_endpoint_setup(NRF_PPI, NRF_PPI_CHANNEL0, eep, tep);
    nrf_ppi_channel_enable(NRF_PPI, NRF_PPI_CHANNEL0);
    nrf_timer_task_trigger(nrf_timer, NRF_TIMER_TASK_CLEAR);
}

void user_button_init() {
    nrf_gpio_cfg_input(BTN_PIN, NRF_GPIO_PIN_PULLUP);
    nrf_gpiote_event_configure(NRF_GPIOTE, 3, BTN_PIN,
                               NRF_GPIOTE_POLARITY_HITOLO);
    nrf_gpiote_int_enable(NRF_GPIOTE, NRF_GPIOTE_INT_IN3_MASK);
    nrf_gpiote_event_enable(NRF_GPIOTE, 3);
    NVIC_EnableIRQ(GPIOTE_IRQn);
}

int main(void) {
    SCB->VTOR = 0x00001000UL;  // must match FLASH ORIGIN in the linker script
    __DSB();
    __ISB();
    timer_led_init();
    user_button_init();
    // Status_LED_Set_Mode(STATUS_LED_MODE_BLINKING_NORMAL);
    app_run();
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

void GPIOTE_IRQHandler() {
    if (nrf_gpiote_event_check(NRF_GPIOTE, NRF_GPIOTE_EVENT_IN_3)) {
        nrf_gpiote_event_clear(NRF_GPIOTE, NRF_GPIOTE_EVENT_IN_3);
        UserButtonState = BUTTON_PRESSED;
    }
}

void Error_Handler(void) {
    __disable_irq();
    Status_LED_Set_Mode(STATUS_LED_MODE_BLINKING_SPECIAL);
    while (1) {
    }
}