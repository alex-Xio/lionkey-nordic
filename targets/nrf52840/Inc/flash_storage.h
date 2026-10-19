#ifndef LIONKEY_NRF52840_FLASH_STORAGE_H
#define LIONKEY_NRF52840_FLASH_STORAGE_H

#include "ctap_storage.h"

typedef struct nrf52840_flash_storage_context {
    uint32_t write_address;
    uint32_t counter_write_address;
} nrf52840_flash_storage_context_t;

#define nrf52840_FLASH_STORAGE_CONST_INIT(context_ptr)                         \
    {                                                                          \
        .context = (context_ptr),                                              \
        .init = nrf52840_flash_storage_init,                                   \
        .find_item = nrf52840_flash_storage_find_item,                         \
        .create_or_update_item = nrf52840_flash_storage_create_or_update_item, \
        .delete_item = nrf52840_flash_storage_delete_item,                     \
        .increment_counter = nrf52840_flash_storage_increment_counter,         \
        .estimate_num_remaining_items =                                        \
            nrf52840_flash_storage_estimate_num_remaining_items,               \
        .erase = nrf52840_flash_storage_erase,                                 \
    }

ctap_storage_status_t nrf52840_flash_storage_init(
    const ctap_storage_t* storage);

ctap_storage_status_t nrf52840_flash_storage_find_item(
    const ctap_storage_t* storage, ctap_storage_item_t* item);

ctap_storage_status_t nrf52840_flash_storage_create_or_update_item(
    const ctap_storage_t* storage, ctap_storage_item_t* item);

ctap_storage_status_t nrf52840_flash_storage_delete_item(
    const ctap_storage_t* storage, uint32_t item_handle);

ctap_storage_status_t nrf52840_flash_storage_increment_counter(
    const ctap_storage_t* storage, uint32_t increment,
    uint32_t* counter_new_value);

size_t nrf52840_flash_storage_estimate_num_remaining_items(
    const ctap_storage_t* storage, const ctap_storage_item_t* item);

ctap_storage_status_t nrf52840_flash_storage_erase(
    const ctap_storage_t* storage);

#endif  // LIONKEY_NRF52840_FLASH_STORAGE_H
