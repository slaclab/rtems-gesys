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
// I386 #include <bsp/irq.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <rtems/rtems_bsdnet.h>
#include <rtems/libio.h>
#include <rtems/tftp.h>

#include <rtems/userenv.h>

#include <cexp.h>

//#include "hack.c"
#include "builddate.c"

#ifdef HAVE_BSPEXT_
#include <bspExt.h>
#endif

#if defined(HAVE_BSP_EXCEPTION_EXTENSION)
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
		BSP_exceptionHandlerInstall(&excExt);
}
#else
#define cexpExcHandlerInstall 0
#endif

#if 0
#include <bsp/vmeUniverse.h>
#endif

#define BOOTPF rtems_bsdnet_bootp_boot_file_name
#define SYSSCRIPT	"st.sys"

static void
cmdline2env(void);

rtems_task Init(
  rtems_task_argument ignored
)
{
char	*symf=0, *user_script=0;
int		argc=4;
int		result=0;
char	*argv[5]={
	"Cexp",	/* program name */
	"-s",
	0,
	SYSSCRIPT,
	0
};


  rtems_libio_set_private_env();

  rtems_bsdnet_initialize_network(); 

  if (rtems_bsdnet_initialize_tftp_filesystem())
	perror("TFTP FS initialization failed");

  printf("This system (ss-20021007) was built on %s\n",system_build_date);

  printf("Trying to synchronize NTP...");
  fflush(stdout);
  if (rtems_bsdnet_synchronize_ntp(0,0)<0)
	printf("FAILED\n");
  else
	printf("OK\n");

  /* stuff command line 'name=value' pairs into the environment */
  cmdline2env();

  cexpInit(cexpExcHandlerInstall);

  if (BOOTPF) {
	char *slash,*dot;

	symf  = malloc(strlen(BOOTPF)+30);
	*symf=0;
	if ('~' != *BOOTPF                /* we are not using rsh */
		&& strncmp(BOOTPF,"/TFTP/",6) /* and it's a relative path */
	   ) {
		/* prepend default server path */
		strcat(symf,"/TFTP/BOOTP_HOST/");
	}

	 strcat(symf,BOOTPF);
	 slash = strrchr(symf,'/');
	 dot   = strrchr(symf,'.');
	 if (slash>dot)
		dot=0;
	 /* substitute suffix */
	 if (dot) {
		strcpy(dot,".sym");
	 } else {
		strcat(symf,".sym");
	 }
	if (slash) {
		int ch=*(slash+1);
		*(slash+1)=0;
		printf("Change Dir to '%s'\n",symf);
		chdir(symf);
		*(slash+1)=ch;
	}
	printf("Trying symfile '%s', system script '%s'\n",symf,SYSSCRIPT);
	argv[2]=symf;
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
  } else {
	fprintf(stderr,"No SYMFILE found\n");
  }
  free(symf);
  free(user_script);
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

static void
cmdline2env(void)
{
char *buf = 0;

char *beg,*end;

	/* make a copy we may modify */
	buf = strdup(rtems_bsdnet_bootp_cmdline);

	/* find 'name=' tags */

	/* this algorithm is copied from the svgm BSP (startup/bspstart.c) */
	for (beg=buf; beg; beg=end) {
		/* skip whitespace */
		while (' '==*beg) {
			if (!*++beg) {
			/* end of string reached; bail out */
				goto done;
			}
		}
		/* simple algorithm to find the end of quoted 'name=quoted'
		 * 			 * substrings. As a side effect, quotes are removed from
		 * 			 			 * the value.
		 * 			 			 			 */
		if ( (end = strchr(beg,'=')) ) {
			char *dst;

			/* eliminate whitespace between variable name and '=' */
			for (dst=end; dst>beg; ) {
					dst--;
					if (!isspace(*dst)) {
						dst++;
						break;
					}
			}

			beg = memmove(beg + (end-dst), beg, dst-beg);

			/* now unquote the value */
			if ('\'' == *++end) {
				/* end points to the 1st char after '=' which is a '\'' */

				dst = end++;

				/* end points to 1st char after '\'' */

				while ('\'' != *end || '\'' == *++end) {
					if ( 0 == (*dst++=*end++) ) {
						/* NO TERMINATING QUOTE FOUND
						 * (for a properly quoted string we
						 * should never get here)
						 */
						end = 0;
						dst--;
						break;
					}
				}
				*dst = 0;
			} else {
				/* first space terminates non-quoted strings */
				if ( (end = strchr(end,' ')) )
					*(end++)=0;
			}
			putenv(beg);
		}

	}
done:
	free(buf);
}
