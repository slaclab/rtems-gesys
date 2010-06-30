#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <bsp.h>

#if RTEMS_VERSION_ATLEAST(4,9,88)
#include <bsp/bootcard.h>
#endif

void
rtemsReboot()
{
#if RTEMS_VERSION_ATLEAST(4,9,99)
	bsp_reset();
#else
	bsp_reset(0);
#endif
}
