#ifndef LIONKEY_NRF52840_MAIN_H
#define LIONKEY_NRF52840_MAIN_H

#include <stdint.h>

extern uint32_t BspButtonState;
typedef enum Status_LED_Mode {
    STATUS_LED_MODE_OFF = 0,
    STATUS_LED_MODE_ON = 1,
    STATUS_LED_MODE_BLINKING_NORMAL = 2,
    STATUS_LED_MODE_BLINKING_SPECIAL = 3,
} Status_LED_Mode;

void Status_LED_Set_Mode(Status_LED_Mode mode);
#define LED_PIN 12

#endif  // LIONKEY_NRF52840_MAIN_H