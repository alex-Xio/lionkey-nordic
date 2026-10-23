#include "flash_storage.h"

#include <string.h>

#include "flash.h"
#include "utils.h"

#define ctap_storage_check(expr)                \
    if ((status = (expr)) != CTAP_STORAGE_OK) { \
        return status;                          \
    }                                           \
    ((void)0)

#define FLASH_STORAGE_VIRGIN_UINT32 (0xFFFFFFFFu)

#define FLASH_WORD_SIZE (16u)
#define FLASH_WORD_NUM_UINT32 (FLASH_WORD_SIZE / 4u)

#define FLASH_STORAGE_VERSION (3u)

#ifndef FLASH_STORAGE_ITEMS_NUM_PAGES
#define FLASH_STORAGE_ITEMS_NUM_PAGES (2u)
#endif
#define FLASH_STORAGE_ITEMS_SIZE \
    (FLASH_STORAGE_ITEMS_NUM_PAGES * FLASH_PAGE_SIZE)

#define FLASH_STORAGE_COUNTER_ENTRY_SIZE (4u)
#define FLASH_STORAGE_COUNTER_ENTRIES_PER_PAGE \
    (FLASH_PAGE_SIZE / FLASH_STORAGE_COUNTER_ENTRY_SIZE)

#define FLASH_ADDRESS_ALIGHNED(addr) (addr % FLASH_WORD_SIZE == 0u)

extern const uint8_t __lionkey_storage_start[];
#define FLASH_STORAGE_BASE ((uint32_t)(uintptr_t)__lionkey_storage_start)

static_assert(FLASH_WORD_SIZE == 16u, "FLASH_WORD_SIZE == 16u");
static_assert(FLASH_PAGE_SIZE % FLASH_WORD_SIZE == 0u,
              "FLASH_PAGE_SIZE % FLASH_WORD_SIZE == 0u");
static_assert(FLASH_STORAGE_ITEMS_NUM_PAGES >= 1u,
              "FLASH_STORAGE_ITEMS_NUM_PAGES >= 1u");

typedef struct LION_ATTR_PACKED sector_header {
    uint32_t version;
    uint32_t word_1;
    uint32_t word_2;
    uint32_t word_3;
} sector_header_t;
static_assert(sizeof(sector_header_t) == FLASH_WORD_SIZE,
              "sizeof(sector_header_t) == FLASH_WORD_SIZE");

typedef struct LION_ATTR_PACKED item_header {
    uint32_t key;
    uint32_t size;
    uint32_t word_2;
    uint32_t word_3;
} item_header_t;
static_assert(sizeof(item_header_t) == FLASH_WORD_SIZE,
              "sizeof(item_header_t) == FLASH_WORD_SIZE");

typedef struct LION_ATTR_PACKED item_checksum {
    uint32_t word_0;
    uint32_t word_1;
    uint32_t word_2;
    uint32_t crc32;
} item_checksum_t;
static_assert(sizeof(item_checksum_t) == FLASH_WORD_SIZE,
              "sizeof(item_checksum_t) == FLASH_WORD_SIZE");

LION_ATTR_ALWAYS_INLINE static inline uint32_t handle_to_address(
    const uint32_t handle) {
    return handle;
}

LION_ATTR_ALWAYS_INLINE static inline uint32_t address_to_handle(
    const uint32_t address) {
    return address;
}

LION_ATTR_ALWAYS_INLINE static inline uint32_t ceil_size_to_boundary(
    const size_t size) {
    static_assert((((0) + 15) & (~0xFu)) == 0, "ceil_size_to_boundary()");
    static_assert((((1) + 15) & (~0xFu)) == 16, "ceil_size_to_boundary()");
    static_assert((((2) + 15) & (~0xFu)) == 16, "ceil_size_to_boundary()");
    static_assert((((3) + 15) & (~0xFu)) == 16, "ceil_size_to_boundary()");
    static_assert((((16) + 15) & (~0xFu)) == 16, "ceil_size_to_boundary()");
    static_assert((((17) + 15) & (~0xFu)) == 32, "ceil_size_to_boundary()");
    static_assert((((20) + 15) & (~0xFu)) == 32, "ceil_size_to_boundary()");
    static_assert((((32) + 15) & (~0xFu)) == 32, "ceil_size_to_boundary()");
    static_assert((((33) + 15) & (~0xFu)) == 48, "ceil_size_to_boundary()");
    return (size + 15) & (~0xFu);
}

LION_ATTR_ALWAYS_INLINE static inline size_t compute_total_item_size(
    const size_t size) {
    return (2 * FLASH_WORD_SIZE) + ceil_size_to_boundary(size) +
           FLASH_WORD_SIZE;
}

LION_ATTR_ALWAYS_INLINE static inline uint32_t is_virgin_word(
    const uint32_t* word) {
    uint32_t virgin_word = FLASH_STORAGE_VIRGIN_UINT32;
    for (int i = 0; i < 4; ++i) {
        virgin_word &= word[i];
    }
    return virgin_word == FLASH_STORAGE_VIRGIN_UINT32;
}

static ctap_storage_status_t erase_page(const uint32_t page_addr) {
    if (!flash_erase_page(page_addr)) {
        error_log(red("flash_erase_page failed") nl);
        return CTAP_STORAGE_ERROR;
    }
    return CTAP_STORAGE_OK;
}

static ctap_storage_status_t program_word(const uint32_t word_addr,
                                          const uint32_t* data) {
    assert(FLASH_ADDRESS_ALIGHNED(word_addr));
    uint32_t tmp[FLASH_WORD_NUM_UINT32];
    memcpy(tmp, data, sizeof(tmp));
    if (!flash_write_words(word_addr, tmp, FLASH_WORD_NUM_UINT32)) {
        error_log(red("flash_write_words failed") nl);
        return CTAP_STORAGE_ERROR;
    }
    return CTAP_STORAGE_OK;
}

LION_ATTR_ALWAYS_INLINE static inline uint32_t items_base(void) {
    return FLASH_STORAGE_BASE;
}

LION_ATTR_ALWAYS_INLINE static inline uint32_t first_item_address(void) {
    // header + delete marker
    return items_base() + 2u * FLASH_WORD_SIZE;
}

LION_ATTR_ALWAYS_INLINE static inline uint32_t counter_page_base(
    const uint32_t page) {
    return items_end() + page * FLASH_PAGE_SIZE;
}
static uint32_t find_write_address(const ctap_storage_t* const storage) {
    ctap_storage_item_t item = {
        .handle = 0u,
        .key = 0u,
    };

    while (nrf52840_flash_storage_find_item(storage, &item) ==
           CTAP_STORAGE_OK) {
        // reset "search criteria" and continue iterating over the items
        item.key = 0u;
    }

    if (item.handle == 0u) {
        return first_item_address();
    }

    return handle_to_address(item.handle) + compute_total_item_size(item.size);
}

static ctap_storage_status_t erase_and_init_pages() {
    ctap_storage_status_t status;

    ctap_storage_check(erase_page(counter_page_base(0u)));
    ctap_storage_check(erase_page(counter_page_base(1u)));

    for (uint32_t page = FLASH_STORAGE_ITEMS_NUM_PAGES; page-- > 0u;) {
        ctap_storage_check(erase_page(items_base() + page * FLASH_PAGE_SIZE));
    }
    const sector_header_t header = {
        .version = FLASH_STORAGE_VERSION,
        .word_1 = 0u,
        .word_2 = 0u,
        .word_3 = 0u,
    };
    ctap_storage_check(program_word(items_base(), &header));

    return CTAP_STORAGE_OK;
}
static uint32_t should_initialize(void) {
    const sector_header_t* const header =
        (const sector_header_t*)(items_base());
    return is_virgin_word((const uint32_t*)header) ||
           header->version != FLASH_STORAGE_VERSION;
}
static uint32_t find_counter_write_address() {
    // find the address of the first 4-byte virgin word 0xFFFFFFFF within the
    // sector
    const uint32_t* value = (const uint32_t*)(counter_page_base(0u));
    const uint32_t* const end = (const uint32_t*)(counter_page_base(2u));
    while (value < end) {
        if (*value == FLASH_STORAGE_VIRGIN_UINT32) {
            break;
        }
        ++value;
    }

    return (uint32_t)value;
}

ctap_storage_status_t nrf52840_flash_storage_init(
    const ctap_storage_t* storage) {
    ctap_storage_status_t status;

    debug_log("nrf52840_flash_storage_init" nl);

    nrf52840_flash_storage_context_t* context = storage->context;

    if (should_initialize()) {
        info_log("flash storage: initialization sequence ..." nl);
        ctap_storage_check(erase_and_init_pages());
        info_log("flash storage: initialized" nl);
    } else {
        debug_log("flash storage: already initialized" nl);
    }

    context->write_address = find_write_address(storage);
    context->counter_write_address = find_counter_write_address();
    return CTAP_STORAGE_OK;
}

LION_ATTR_ALWAYS_INLINE static inline uint32_t items_end(void) {
    return items_base() + FLASH_STORAGE_ITEMS_NUM_PAGES * FLASH_PAGE_SIZE;
}

ctap_storage_status_t nrf52840_flash_storage_find_item(
    const ctap_storage_t* storage, ctap_storage_item_t* item) {
    nrf52840_flash_storage_context_t* context = storage->context;

    const uint32_t min_address = first_item_address();
    const uint32_t max_address = items_end();

    uint32_t address = min_address;

    // continue iteration
    if (item->handle != 0u) {
        address = handle_to_address(item->handle) +
                  compute_total_item_size(item->size);
        assert(min_address <= address && address < max_address);
    }

    while (address < max_address) {
        const item_header_t* const header = (const item_header_t*)(address);
        const uint32_t* const delete_marker =
            (const uint32_t*)(address + FLASH_WORD_SIZE);
        const uint8_t* const data =
            (const uint8_t*)(address + (2 * FLASH_WORD_SIZE));

        if (is_virgin_word((const uint32_t*)header)) {
            break;
        }

        // TODO: check checksum
        // const item_checksum_t *const checksum = (const item_checksum_t *) (
        // 	address + 2 * FLASH_WORD_SIZE +
        // ceil_size_to_boundary(header->size)
        // );

        if (*delete_marker == 0u) {
            if (item->key == 0u || item->key == header->key) {
                item->handle = address_to_handle(address);
                item->key = header->key;
                item->size = header->size;
                item->data = data;
                return CTAP_STORAGE_OK;
            }
        }

        address += compute_total_item_size(header->size);
    }

    return CTAP_STORAGE_ITEM_NOT_FOUND;
}
ctap_storage_status_t nrf52840_flash_storage_delete_item(
    const ctap_storage_t* storage, uint32_t item_handle) {
    nrf52840_flash_storage_context_t* context = storage->context;

    assert(item_handle > 0);

    const uint32_t item_address = handle_to_address(item_handle);

    debug_log(
        "nrf52840_flash_storage_delete_item: item_address = 0x%08" PRIx32 nl,
        item_address);

    const uint32_t min_address = first_item_address();

    assert(min_address <= item_address &&
           item_address < context->write_address);

    const uint32_t delete_marker = 1u;
    return program_word(item_address + FLASH_WORD_SIZE, &delete_marker);
}

ctap_storage_status_t nrf52840_flash_storage_create_item(
    const ctap_storage_t* storage, ctap_storage_item_t* item) {
    nrf52840_flash_storage_context_t* context = storage->context;

    assert(item->key != 0u);
    assert(item->size == 0u || item->data != NULL);

    const uint32_t total_item_size = compute_total_item_size(item->size);
    const uint32_t new_write_address = context->write_address + total_item_size;

    if (new_write_address > items_end()) {
        error_log(red("not enough space to write item") nl);
        return CTAP_STORAGE_OUT_OF_MEMORY_ERROR;
    }

    const item_header_t header = {
        .key = item->key,
        .size = item->size,
        .word_2 = 0u,
        .word_3 = 0u,
    };
    ctap_storage_status_t status;
    ctap_storage_check(program_word(context->write_address, &header));

    const uint32_t delete_marker = 0u;
    ctap_storage_check(
        program_word(context->write_address + FLASH_WORD_SIZE, &delete_marker));

    ctap_storage_check(
        program_word(context->write_address + (2 * FLASH_WORD_SIZE),
                     (const uint32_t*)item->data));
    context->write_address += total_item_size;
    return CTAP_STORAGE_OK;
}