#include "flash.h"

#include "nrf_nvmc.h"

void nvmc_wait_ready() {
    while (!nrf_nvmc_ready_check(NRF_NVMC)) {
    }
}
bool flash_erase_page(uint32_t page_address) {
    if ((page_address % FLASH_PAGE_SIZE) != 0u) {
        return false;
    }
    nrf_nvmc_mode_set(NRF_NVMC, NRF_NVMC_MODE_ERASE);
    nvmc_wait_ready();

    nrf_nvmc_page_erase_start(NRF_NVMC, page_address);
    nvmc_wait_ready();

    nrf_nvmc_mode_set(NRF_NVMC, NRF_NVMC_MODE_READONLY);
    nvmc_wait_ready();

    const volatile uint32_t* const page =
        (const volatile uint32_t*)(uintptr_t)page_address;
    for (size_t i = 0; i < FLASH_PAGE_SIZE / sizeof(uint32_t); i++) {
        if (page[i] != 0xFFFFFFFFu) {
            return false;
        }
    }

    return true;
}

bool flash_write_words(uint32_t address, const uint32_t* words,
                       size_t num_words) {
    if ((address & 3u) != 0u) {
        return false;
    }
    volatile uint32_t* const addr = (volatile uint32_t*)(uintptr_t)address;
    nrf_nvmc_mode_set(NRF_NVMC, NRF_NVMC_MODE_WRITE);
    nvmc_wait_ready();

    for (size_t i = 0; i < num_words; i++) {
        addr[i] = words[i];
        nvmc_wait_ready();
    }

    nrf_nvmc_mode_set(NRF_NVMC, NRF_NVMC_MODE_READONLY);
    nvmc_wait_ready();

    for (size_t i = 0; i < num_words; i++) {
        if (addr[i] != words[i]) {
            return false;
        }
    }
}
