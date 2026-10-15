/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x00001000, LENGTH = 0x000DB000
  RAM (rwx) : ORIGIN = 0x000DC000, LENGTH = 0x0004000
  CODE_RAM (rwx) : ORIGIN = 0x20000008, LENGTH = 0x0003FF8
}


INCLUDE "nrf_common.ld"
