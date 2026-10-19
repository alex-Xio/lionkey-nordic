#ifndef LIONKEY_NRF52840_FLASH_H
#define LIONKEY_NRF52840_FLASH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// nRF52840 has 1 MB of flash that is devided into 256 pages of 4 KB size
//
// Things differences from the STM32H533 flash.h:
// No ECC.
// Writes can only clear bits. Only an erase sets 1s.
// No read-while-write. The CPU stalls while the NVMC is busy.
//   Erase delays all interrupts, including USB.

#define FLASH_PAGE_SIZE 4096u

// Erases one page. page_address must be page aligned.
// Returns true if the whole page reads back as 0xFF afterwards.
bool flash_erase_page(uint32_t page_address);

// Writes num_words 32-bit words starting at address (must be 4-byte aligned).
// Words are programmed in ascending order. Returns true if the data reads back
// as written.
bool flash_write_words(uint32_t address, const uint32_t* words,
                       size_t num_words);

#endif  // LIONKEY_NRF52840_FLASH_H
