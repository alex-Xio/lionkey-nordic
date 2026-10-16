#ifndef LIONKEY_NRF52840_MAIN_H
#define LIONKEY_NRF52840_MAIN_H

#include "nrf_gpio.h"

typedef enum ButtonState {
    BUTTON_RELEASED = 0,
    BUTTON_PRESSED = 1,
} ButtonState;

extern volatile ButtonState UserButtonState;
typedef enum Status_LED_Mode {
    STATUS_LED_MODE_OFF = 0,
    STATUS_LED_MODE_ON = 1,
    STATUS_LED_MODE_BLINKING_NORMAL = 2,
    STATUS_LED_MODE_BLINKING_SPECIAL = 3,
} Status_LED_Mode;

void Status_LED_Set_Mode(Status_LED_Mode mode);

#define LED_PIN NRF_GPIO_PIN_MAP(0, 12)
#define BTN_PIN NRF_GPIO_PIN_MAP(1, 6)

#endif  // LIONKEY_NRF52840_MAIN_H