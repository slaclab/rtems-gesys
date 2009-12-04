#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>

uint32_t BSP_sbrk_policy = -1;

#ifdef RTEMS_CEXP_TEXT_REGION_SIZE
/* hopefully goes into .bss */
char cexpTextRegion[RTEMS_CEXP_TEXT_REGION_SIZE] = {0};
unsigned long cexpTextRegionSize = RTEMS_CEXP_TEXT_REGION_SIZE;
#endif
