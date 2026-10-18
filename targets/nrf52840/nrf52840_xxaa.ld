/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc)

MEMORY
{
  FLASH   (rx)  : ORIGIN = 0x00001000, LENGTH = 0x000DB000
  STORAGE (r)   : ORIGIN = 0x000DC000, LENGTH = 0x00004000 /*4 pages for lionkey storage*/
  RAM     (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00040000
}

__lionkey_storage_start = ORIGIN(STORAGE);
__lionkey_storage_end   = ORIGIN(STORAGE) + LENGTH(STORAGE);

ASSERT((ORIGIN(FLASH) + LENGTH(FLASH)) <= ORIGIN(STORAGE), "app flash overlaps storage")
ASSERT(__lionkey_storage_end <= 0x000E0000, "storage overlaps the DFU bootloader")
ASSERT((__lionkey_storage_start % 4096) == 0, "storage must be page aligned")


INCLUDE "nrf_common.ld"
