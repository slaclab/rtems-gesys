/*
 * Copyright (c) 2013-2014 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Dornierstr. 4
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/stat.h>
#include <sys/socket.h>

#include <net/if.h>

#include <assert.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>

#include <machine/rtems-bsd-commands.h>

#include <rtems.h>
#include <rtems/stackchk.h>
#include <rtems/bsd/bsd.h>

static void
default_network_set_self_prio(rtems_task_priority prio)
{
	rtems_status_code sc;

	sc = rtems_task_set_priority(RTEMS_SELF, prio, &prio);
	assert(sc == RTEMS_SUCCESSFUL);
}

static void
default_network_ifconfig_lo0(void)
{
	int exit_code;
	char *lo0[] = {
		"ifconfig",
		"lo0",
		"inet",
		"127.0.0.1",
		"netmask",
		"255.255.255.0",
		NULL
	};
	char *lo0_inet6[] = {
		"ifconfig",
		"lo0",
		"inet6",
		"::1",
		"prefixlen",
		"128",
		"alias",
		NULL
	};

	exit_code = rtems_bsd_command_ifconfig(RTEMS_BSD_ARGC(lo0), lo0);
	assert(exit_code == EX_OK);

	exit_code = rtems_bsd_command_ifconfig(RTEMS_BSD_ARGC(lo0_inet6), lo0_inet6);
	assert(exit_code == EX_OK);
}

static void
default_network_ifconfig_hwif0(char *ifname)
{
	int exit_code;
	char *ifcfg[] = {
		"ifconfig",
		ifname,
		"up",
		NULL
	};

	exit_code = rtems_bsd_command_ifconfig(RTEMS_BSD_ARGC(ifcfg), ifcfg);
	assert(exit_code == EX_OK);
}

static void
default_network_route_hwif0(char *ifname)
{
}

static void
default_network_dhcpcd_task(rtems_task_argument arg)
{
	int exit_code;
	char *dhcpcd[] = {
		"dhcpcd",
        "cgem0",
		NULL
	};

	(void)arg;

#ifdef DEFAULT_NETWORK_DHCPCD_NO_DHCP_DISCOVERY
	static const char cfg[] = "nodhcp\nnodhcp6\n";
	int fd;
	int rv;
	ssize_t n;

	fd = open("/etc/dhcpcd.conf", O_CREAT | O_WRONLY,
	    S_IRWXU | S_IRWXG | S_IRWXO);
	assert(fd >= 0);

	n = write(fd, cfg, sizeof(cfg));
	assert(n == (ssize_t) sizeof(cfg));

	rv = close(fd);
	assert(rv == 0);
#endif

	exit_code = rtems_bsd_command_dhcpcd(RTEMS_BSD_ARGC(dhcpcd), dhcpcd);
	assert(exit_code == EXIT_SUCCESS);
}

static void
default_network_dhcpcd(void)
{
	rtems_status_code sc;
	rtems_id id;

	sc = rtems_task_create(
		rtems_build_name('D', 'H', 'C', 'P'),
		RTEMS_MAXIMUM_PRIORITY - 1,
		2 * RTEMS_MINIMUM_STACK_SIZE,
		RTEMS_DEFAULT_MODES,
		RTEMS_FLOATING_POINT,
		&id
	);
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_task_start(id, default_network_dhcpcd_task, 0);
	assert(sc == RTEMS_SUCCESSFUL);
}

static void
default_network_on_exit(int exit_code, void *arg)
{
	rtems_printer printer;

	(void)arg;

	rtems_print_printer_printf(&printer);
	rtems_stack_checker_report_usage_with_plugin(&printer);

	puts("*** Network Exit ***\n");
}

int
gesys_network_start()
{
	rtems_status_code sc;
	char ifnamebuf[IF_NAMESIZE];
	char *ifname;

	on_exit(default_network_on_exit, NULL);

	/* Let other tasks run to complete background work */
	default_network_set_self_prio(RTEMS_MAXIMUM_PRIORITY - 1U);

	rtems_bsd_initialize();

	ifname = if_indextoname(1, &ifnamebuf[0]);
	assert(ifname != NULL);

	/* Let the callout timer allocate its resources */
	sc = rtems_task_wake_after(2);
	assert(sc == RTEMS_SUCCESSFUL);

	default_network_ifconfig_lo0();
	default_network_ifconfig_hwif0(ifname);
	default_network_route_hwif0(ifname);
	default_network_dhcpcd();

	return 0;
}

