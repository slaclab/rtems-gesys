/*  Init
 *
 *  Initialization task for a generic RTEMS/CEXP system
 *
 *  Author: Till Straumann <strauman@slac.stanford.edu>
 *
 *  $Id$
 */

/*
 * Copyright 2002,2003, Stanford University and
 * 		Till Straumann <strauman@@slac.stanford.edu>
 * 
 * Stanford Notice
 * ***************
 * 
 * Acknowledgement of sponsorship
 * * * * * * * * * * * * * * * * *
 * This software was produced by the Stanford Linear Accelerator Center,
 * Stanford University, under Contract DE-AC03-76SFO0515 with the Department
 * of Energy.
 * 
 * Government disclaimer of liability
 * - - - - - - - - - - - - - - - - -
 * Neither the United States nor the United States Department of Energy,
 * nor any of their employees, makes any warranty, express or implied,
 * or assumes any legal liability or responsibility for the accuracy,
 * completeness, or usefulness of any data, apparatus, product, or process
 * disclosed, or represents that its use would not infringe privately
 * owned rights.
 * 
 * Stanford disclaimer of liability
 * - - - - - - - - - - - - - - - - -
 * Stanford University makes no representations or warranties, express or
 * implied, nor assumes any liability for the use of this software.
 * 
 * This product is subject to the EPICS open license
 * - - - - - - - - - - - - - - - - - - - - - - - - - 
 * Consult the LICENSE file or http://www.aps.anl.gov/epics/license/open.php
 * for more information.
 * 
 * Maintenance of notice
 * - - - - - - - - - - -
 * In the interest of clarity regarding the origin and status of this
 * software, Stanford University requests that any recipient of it maintain
 * this notice affixed to any distribution by the recipient that contains a
 * copy or derivative of this software.
 */


/*
 *  The initialization task performs the following steps:
 *
 *   - initialize networking
 *   - mount the TFTP filesystem on '/TFTP/'
 *   - synchronize with NTP server
 *   - initialize CEXP
 *   - retrieve the boot file name using
 *
 *     rtems_bsdnet_bootp_boot_file_name
 *
 *     i.e. the file name obtained by RTEMS/BOOTP
 *     from a BOOTP/DHCP server.
 *
 *     if the file name is relative, "/TFTP/BOOTP_HOST/"
 *     is prepended.
 *
 *   - substitute the boot file's extension by ".sym"
 *     (e.g. sss/yyy/z.exe becomes /TFTP/BOOTP_HOST/sss/yyy/z.sym)
 *
 *   - retrieve 'name=value' pairs from the command line
 *     string (BOOTP/DHCP option 129) and stick them
 *     into the environment.
 *
 *   - chdir into the directory where the boot (and symbol)
 *     files reside.
 *
 *   - invoke 'cexp("-s <symfile> st.sys")', i.e. start
 *     CEXP and try to load the symbol table and subsequently
 *     execute a 'st.sys' file AKA 'system script'.
 *
 *     If loading the symbol file fails, the user is
 *     prompted for a path and loading the symbol table
 *     reattempted (repeated until success).
 *
 *   - try to 'getenv("INIT")', thus retrieve the name
 *     of a 'user script' from the command line.
 *    
 *     (e.g. dhcpd.conf provides:
 *          option cmdline code 129 = text;
 *          ...
 *     		option cmdline "INIT=/TFTP/BOOTP_HOST/epics/iocxxx/st.cmd"
 *     )
 *
 *     If getenv() succeeds, 'chdir' to the directory
 *     where the user script resides and execute the
 *     user (CEXP) script.
 *
 *   - eventually, CEXP enters interactive mode at the console.
 *
 */
// I386 #include <bsp/irq.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <syslog.h>

#include <rtems/rtems_bsdnet.h>
#include <rtems/libio.h>
#include <rtems/tftp.h>

#include <rtems/userenv.h>

#include <cexp.h>

#ifdef USE_TECLA
#include <libtecla.h>
#else
#define del_GetLine(gl)					free(gl)
#define	new_GetLine(len,lines)			malloc(len)
#define gl_configure_getline(gl,a,b,c)	do {} while(0)
typedef char GetLine;
static	char *my_getline(char *rval, char *prompt, int len);
#endif

#define LINE_LENGTH 200

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

#define BOOTPF rtems_bsdnet_bootp_boot_file_name
#define SYSSCRIPT	"st.sys"

static void
cmdline2env(void);

#ifdef USE_TECLA
int
ansiTiocGwinszInstall(int slot);
#endif

rtems_task Init(
  rtems_task_argument ignored
)
{
GetLine	*gl=0;
char	*symf=0, *user_script=0, *bufp;
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

#ifdef STACK_CHECKER_ON
  {
	extern void Stack_check_Initialize();
	Stack_check_Initialize();
  }
#endif

#ifdef HAVE_BSPEXT_
  bspExtInit();
#endif

  rtems_bsdnet_initialize_network(); 

  if (rtems_bsdnet_initialize_tftp_filesystem())
	perror("TFTP FS initialization failed");

  printf("$Id$\n");
  printf("Welcome to RTEMS GeSys\n");
  printf("This system $Name$ was built on %s\n",system_build_date);

  printf("Trying to synchronize NTP...");
  fflush(stdout);
  if (rtems_bsdnet_synchronize_ntp(0,0)<0)
	printf("FAILED\n");
  else
	printf("OK\n");

  /* remote logging only works after a call to openlog()... */
  openlog(0, LOG_PID | LOG_CONS, 0); /* use RTEMS defaults */

#if defined(USE_TECLA)
  /*
   * Install our special line discipline which implements
   * TIOCGWINSZ
   */
  printf("Installing TIOCGWINSZ line discipline: %s.\n",
		 ansiTiocGwinszInstall(7) ? "failed" : "ok");
#endif

  /* stuff command line 'name=value' pairs into the environment */
  cmdline2env();

  cexpInit(cexpExcHandlerInstall);

  bufp = BOOTPF;

  while (1) {

  if (!bufp) {
	if (!gl) {
		assert( gl = new_GetLine(LINE_LENGTH,10) );
		/* silence warnings about missing .teclarc */
		gl_configure_getline(gl,0,0,0);
	}
	do {
#ifdef USE_TECLA
		bufp = gl_get_line(gl, "Enter Symbol File Name: ", NULL, 0);
#else
		bufp = my_getline(gl, "Enter Symbol File Name: ", LINE_LENGTH);
#endif
	} while (!bufp || !*bufp);
  }

  {
	char *slash,*dot;

	symf  = realloc(symf, strlen(bufp)+30);
	*symf=0;
	if ('~' != *bufp                /* we are not using rsh */
		&& strncmp(bufp,"/TFTP/",6) /* and it's a relative path */
	   ) {
		/* prepend default server path */
		strcat(symf,"/TFTP/BOOTP_HOST/");
	}

	 strcat(symf,bufp);
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
  		free(symf); symf=0;
		if (gl) {
			del_GetLine(gl);
			gl = 0;
		}
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
  			free(user_script); user_script=0;
		} while (!result || CEXP_MAIN_NO_SCRIPT==result);
	}
  }

  free(symf);        symf=0;
  free(user_script); user_script=0;

  switch (result) {
		case CEXP_MAIN_NO_SYMS  :
				fprintf(stderr,"CEXP_MAIN_NO_SYMS\n");
				bufp = 0;
				continue; /* retry asking them for a symbol file */
		break;
		case CEXP_MAIN_INVAL_ARG: fprintf(stderr,"CEXP_MAIN_INVAL_ARG\n"); break;
		case CEXP_MAIN_NO_SCRIPT: fprintf(stderr,"CEXP_MAIN_NO_SCRIPT\n"); break;
		case CEXP_MAIN_KILLED   : fprintf(stderr,"CEXP_MAIN_KILLED\n");    break;
		case CEXP_MAIN_NO_MEM   : fprintf(stderr,"CEXP_MAIN_NO_MEM\n");    break;

	default:
		if (result)
			fprintf(stderr,"unknown error\n");
  }

  } /* while (1), retry asking for a sym-filename */
  if (gl) {
	del_GetLine(gl);
	gl = 0;
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
		 * substrings. As a side effect, quotes are removed from
		 * the value.
		 */
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

#ifndef USE_TECLA
static char *my_getline(char *rval, char *prompt, int len)
{
int		ch;
char	*cp;

    if (prompt)
        fputs(prompt,stdout);

    for (cp=rval; cp<rval+len-1 && (ch=getchar())>=0;) {
            switch (ch) {
                case '\n': goto done;
                case '\b':
                    if (cp>rval) {
                        cp--;
                        fputs("\b ",stdout);
                    }
                    break;
                default:
                    *cp++=ch;
                    break;
            }
    }
done:
	*cp=0;
    return rval;
}
#endif
