/*
 * RTEMS network configuration for EPICS
 *  rtems_netconfig.c,v 1.1 2001/08/09 17:54:05 norume Exp
 *      Author: W. Eric Norum
 *              eric.norum@usask.ca
 *              (306) 966-5394
 *
 * This file can be copied to an application source dirctory
 * and modified to override the values shown below.
 */
#include <stdio.h>
#include <bsp.h>
#include <rtems/rtems_bsdnet.h>

#define NETWORK_TASK_PRIORITY           90

#ifdef MULTI_NETDRIVER

extern int rtems_3c509_driver_attach (struct rtems_bsdnet_ifconfig *, int);
extern int rtems_fxp_attach (struct rtems_bsdnet_ifconfig *, int);
extern int rtems_elnk_driver_attach (struct rtems_bsdnet_ifconfig *, int);
extern int rtems_dec21140_driver_attach (struct rtems_bsdnet_ifconfig *, int);

/* these don't probe and will be used even if there's no device :-( */
extern int rtems_ne_driver_attach (struct rtems_bsdnet_ifconfig *, int);
extern int rtems_wd_driver_attach (struct rtems_bsdnet_ifconfig *, int);

static struct rtems_bsdnet_ifconfig isa_netdriver_config[] = {
	{
		"ep0", rtems_3c509_driver_attach, isa_netdriver_config + 1,
	},
	{
		"ne1", rtems_ne_driver_attach, 0, irno: 9 /* qemu cannot configure irq-no :-(; has it hardwired to 9 */
	},
};

static struct rtems_bsdnet_ifconfig pci_netdriver_config[]={
	{
	"fxp1", rtems_fxp_attach, pci_netdriver_config+1,
	},
	{
	"dc1", rtems_dec21140_driver_attach, pci_netdriver_config+2,
	},
	{
	"elnk1", rtems_elnk_driver_attach, isa_netdriver_config,
	},
};

static int pci_check(struct rtems_bsdnet_ifconfig *cfg, int attaching)
{
	if ( attaching ) {
		cfg->next = pcib_init() ? isa_netdriver_config : pci_netdriver_config;
	}
	return -1;
}

static struct rtems_bsdnet_ifconfig netdriver_config[]={
	{
		"dummy", pci_check, 0
	}
};

#else


/*
 * The following conditionals select the network interface card.
 *
 * By default the network interface specified by the board-support
 * package is used.
 * To use a different NIC for a particular application, copy this file to the
 * application directory and add the appropriate -Dxxxx to the compiler flag.
 * To specify a different NIC on a site-wide basis, add the appropriate
 * flags to the site configuration file for the target.  For example, to
 * specify a 3COM 3C509 for all RTEMS-pc386 targets at your site, add
 *      TARGET_CFLAGS += -DEPICS_RTEMS_NIC_3C509
 * to configure/os/CONFIG_SITE.Common.RTEMS-pc386.
 */
#if defined(EPICS_RTEMS_NIC_3C509)            /* 3COM 3C509 */
  extern int rtems_3c509_driver_attach (struct rtems_bsdnet_ifconfig *, int);
# define NIC_NAME   "ep0"
# define NIC_ATTACH rtems_3c509_driver_attach

#elif defined(RTEMS_BSP_NETWORK_DRIVER_NAME)  /* Use NIC provided by BSP */
# define NIC_NAME   RTEMS_BSP_NETWORK_DRIVER_NAME
# define NIC_ATTACH RTEMS_BSP_NETWORK_DRIVER_ATTACH
/* T. Straumann, 12/18/2001, added declaration, just in case */
extern int
RTEMS_BSP_NETWORK_DRIVER_ATTACH();
#endif

#ifdef NIC_NAME
static struct rtems_bsdnet_ifconfig netdriver_config[1] = {{
    NIC_NAME,	/* name */
    NIC_ATTACH,	/* attach function */
    0,			/* link to next interface */
}};
#else
#warning "NO KNOWN NETWORK DRIVER FOR THIS BSP -- YOU MAY HAVE TO EDIT rtems_netconfig.c"
#define netdriver_config 0
#endif

#endif

extern void rtems_bsdnet_loopattach();
static struct rtems_bsdnet_ifconfig loopback_config = {
    "lo0",                          /* name */
    (int (*)(struct rtems_bsdnet_ifconfig *, int))rtems_bsdnet_loopattach, /* attach function */
    netdriver_config,               /* link to next interface */
    "127.0.0.1",                    /* IP address */
    "255.0.0.0",                    /* IP net mask */
};

struct rtems_bsdnet_config rtems_bsdnet_config = {
    &loopback_config,         /* Network interface */
    netdriver_config ? rtems_bsdnet_do_bootp : 0,    /* Use BOOTP to get network configuration */
    NETWORK_TASK_PRIORITY,    /* Network task priority */
#ifdef MEMORY_SCARCE
    100*1024,                 /* MBUF space */
    200*1024,                 /* MBUF cluster space */
#elif defined(MEMORY_HUGE)
    2*1024*1024,                 /* MBUF space */
    5*1024*1024,                 /* MBUF cluster space */
#else
    180*1024,                 /* MBUF space */
    350*1024,                 /* MBUF cluster space */
#endif
};
