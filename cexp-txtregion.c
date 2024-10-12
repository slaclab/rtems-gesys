#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>

uintptr_t BSP_sbrk_policy = -1; /*-1;*/

#if defined(RTEMS_CEXP_TEXT_REGION_SIZE) && RTEMS_CEXP_TEXT_REGION_SIZE > 0
/* hopefully goes into .bss */
char cexpTextRegion[RTEMS_CEXP_TEXT_REGION_SIZE] = {0};
unsigned long cexpTextRegionSize = RTEMS_CEXP_TEXT_REGION_SIZE;
#else
/** JL: workaround for GCC 4.8.5 bug: we must define cexpTextRegionSize here, otherwise the weak alias cexpTextRegionSize in cexpsh will be used
 * The symbol that cexpTextRegionSize is aliased to ends up in .rodata due to an optimization bug. This all works fine with -O0
 */
unsigned long cexpTextRegionSize = 0;
#endif
