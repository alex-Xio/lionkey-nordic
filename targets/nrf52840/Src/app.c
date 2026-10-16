#include "app.h"

#include "main.h"
#include "usb.h"

static void app_init(void) {
    // Status_LED_Set_Mode(STATUS_LED_MODE_ON)

    //     // const uint32_t t1 = HAL_GetTick();
    // ctaphid_init(&app_ctaphid);
    // if (app_storage.init(&app_storage) != CTAP_STORAGE_OK) {
    //     Error_Handler();
    // }
    // // if (app_sw_crypto.init(&app_sw_crypto, 0) != CTAP_CRYPTO_OK) {
    // // 	Error_Handler();
    // // }
    // if (app_hw_crypto.init(&app_hw_crypto, 0) != CTAP_CRYPTO_OK) {
    //     Error_Handler();
    // }
    // ctap_init(&app_ctap);
    // // const uint32_t t2 = HAL_GetTick();

    // info_log("init done in %" PRId32 " ms" nl, t2 - t1);
}

noreturn void app_run(void) {
    info_log(nl nl cyan("app_run") nl);
    app_init();
    usb_init();
}