/* Unix platform.h for EK */
/* File input buffer size */
#ifndef IBUFLEN
#define IBUFLEN  128
#endif /* IBUFLEN */

#include "../include/target.h"
#define KERMIT_FLASH_START WOLFBOOT_PARTITION_BOOT_ADDRESS
#define KERMIT_FLASH_SIZE WOLFBOOT_PARTITION_SIZE
#ifndef OBUFLEN
#define OBUFLEN  WOLFBOOT_SECTOR_SIZE
#endif

#define NO_SSW
#define NO_AT
#define NO_SCAN
#define FN_MAX  16
#define RECVONLY