/*  Init
 *
 *  This routine is the initialization task for this test program.
 *  It is called from init_exec and has the responsibility for creating
 *  and starting the tasks that make up the test.  If the time of day
 *  clock is required for the test, it should also be set to a known
 *  value by this function.
 *
 *  Input parameters:  NONE
 *
 *  Output parameters:  NONE
 *
 *  COPYRIGHT (c) 1989-1999.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.OARcorp.com/rtems/license.html.
 *
 *  $Id$
 */
#include <bsp.h>
#include <bsp/irq.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <rtems/rtems_bsdnet.h>
#include <rtems/libio.h>
#include <rtems/tftp.h>

#include <rtems/userenv.h>

#include <cexp.h>

#ifdef HAVE_BSPEXT_
#include <bspExt.h>
#endif

#define HAVE_BSP_EXCEPTION_EXTENSION
#ifdef HAVE_BSP_EXCEPTION_EXTENSION
#include <bsp/bspException.h>

static void
cexpExcHandler(BSP_ExceptionExtension ext)
{
		cexp_kill(0);
}

static BSP_ExceptionExtensionRec excExt={
		0, /* no lowlevel handler */
		0, /* be verbose (not quiet) */
		cexpExcHandler
};

static void
cexpExcHandlerInstall(void (*handler)(int))
{
		/* we use cexp_kill() directly; no need
		 * for the handler arg
		 */
		rtems_task_set_note(
				RTEMS_SELF,
				BSP_EXCEPTION_NOTEPAD,
				(rtems_unsigned32)&excExt);
}
#else
#define cexpExcHandlerInstall 0
#endif

#include <bsp/vmeUniverse.h>

#define BOOTPF rtems_bsdnet_bootp_boot_file_name
#define SYSSCRIPT	"st.sys"

extern cexpFlashSymLoad();

rtems_task Init(
  rtems_task_argument ignored
)
{
char	*user_script=0;
int		argc=4;
int		result=0;
char	*argv[5]={
	"Cexp",	/* program name */
	SYSSCRIPT,
	0
};
char	*slash;


  rtems_libio_set_private_env();

  cexpInit(cexpExcHandlerInstall);

  cexpFlashSymLoad(2);

  if (!(result=cexp_main(argc,argv)) || CEXP_MAIN_NO_SCRIPT==result) {
		/* try a user script */
		if ((user_script=getenv("INIT"))) {
			printf("Trying user script '%s':\n",user_script);
			argv[1]=user_script=strdup(user_script);
			if ((slash = strrchr(user_script,'/'))) {
				/* chdir to where the user script resides */
				int ch=*(++slash);
				*(slash)=0;
				printf("Change Dir to '%s'\n",user_script);
				chdir(user_script);
				*(slash)=ch;
				argv[1]=slash;
			}
			argc=2;
		} else {
			argc=1;
		}
		do {
			result=cexp_main(argc,argv);
			argc=1;
		} while (!result || CEXP_MAIN_NO_SCRIPT==result);
  }
  switch (result) {
		case CEXP_MAIN_INVAL_ARG: fprintf(stderr,"CEXP_MAIN_INVAL_ARG\n"); break;
		case CEXP_MAIN_NO_SYMS  : fprintf(stderr,"CEXP_MAIN_NO_SYMS\n");   break;
		case CEXP_MAIN_NO_SCRIPT: fprintf(stderr,"CEXP_MAIN_NO_SCRIPT\n"); break;
		case CEXP_MAIN_KILLED   : fprintf(stderr,"CEXP_MAIN_KILLED\n");    break;
		case CEXP_MAIN_NO_MEM   : fprintf(stderr,"CEXP_MAIN_NO_MEM\n");    break;

	default:
		if (result)
			fprintf(stderr,"unknown error\n");
  }
  fprintf(stderr,"Unable to execute CEXP - suspending initialization...\n");
  rtems_task_suspend(RTEMS_SELF);
  exit( 1 );
}
