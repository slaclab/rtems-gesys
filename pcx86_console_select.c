#include <rtems.h>
#include <bsp.h>
#include <uart.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(HAVE_BSP_CMDLINE) || defined(HAVE_BSP_COMMANDLINE_STRING)
void
BSP_runtime_console_select(int *pConsolePort, int *pPrintkPort)
{
#if   defined(HAVE_BSP_CMDLINE)
const char *cmdline = bsp_cmdline();
#elif defined(HAVE_BSP_COMMANDLINE_STRING)
const char *cmdline = BSP_commandline_string;
#endif

	if ( 0 != strstr(cmdline, "CONSOLE=COM1") ) {
		*pPrintkPort = *pConsolePort = BSP_CONSOLE_PORT_COM1;
	} else if ( 0 != strstr(cmdline, "CONSOLE=COM2") ) {
		/* cannot sent printk to COM2; use 'hard-configuration' for printk */
		*pConsolePort = BSP_CONSOLE_PORT_COM2;
	} else if ( 0 != strstr(cmdline, "CONSOLE=CONSOLE") ) {
		*pPrintkPort = *pConsolePort = BSP_CONSOLE_PORT_CONSOLE;
	} /* else use 'hard' configuration */
}
#endif
