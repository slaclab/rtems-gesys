m4_include([m4/cvstag.m4])

AC_INIT(GeSys, TILLAC_GITTAG, <strauman@slac.stanford.edu>)

TILLAC_CHECK_GITTAG

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([foreign 1.10])
AM_MAINTAINER_MODE

TILLAC_RTEMS_CHECK_TOOLS

# Must add this BEFORE TILLAC_RTEMS_SETUP
# so that the dummy-top 'config.status' also
# knows how to make a config.h
AM_CONFIG_HEADER(config.h)

TILLAC_RTEMS_SETUP

# Check for additional programs we need for building
RTEMS_CHECK_TOOL([NM],nm)
AC_ARG_VAR([OBJCOPY],[objcopy binutils tool])
AC_ARG_VAR([OBJDUMP],[objdump binutils tool])
RTEMS_CHECK_TOOL([OBJDUMP],objdump)
AC_ARG_VAR([READELF],[readelf binutils tool])
RTEMS_CHECK_TOOL([READELF],readelf)
AC_ARG_VAR([LDEP],[library dependency tool])
RTEMS_CHECK_PROG([LDEP],ldep,ldep)

# if gesys is built as part of the whole ssrlApps
# package then we cannot check for the presence of
# individual libraries since they do not exist yet
# at configure time.
if test -f ../ssrlApps.components ; then
	tillac_rtems_inside_package=yes
. ../ssrlApps.components
else
	tillac_rtems_inside_package=no
fi
AC_DEFUN([SSRLAPPS_CHECK_COMPONENT],
	[AC_MSG_CHECKING([if $1 was built as part of ssrlApps])
	 if echo "$ssrlapps_components" | grep '\<$1\>' > /dev/null ; then
		AC_MSG_RESULT([Yes])
		true
	 else
		AC_MSG_RESULT([No])
		false
	 fi]dnl
)

AC_ARG_ENABLE(tecla,
	AC_HELP_STRING([--disable-tecla],
		[disable using the tecla library])
)

AC_ARG_ENABLE(loader,
	AC_HELP_STRING([--disable-loader],
		[disable the CEXP loader])
)

AC_ARG_ENABLE(netboot,
	AC_HELP_STRING([--disable-netboot],
		[disable using the netboot library])
)

AC_ARG_ENABLE(nfs,
	AC_HELP_STRING([--disable-nfs],
		[disable use of NFS])
)

AC_ARG_ENABLE(tftpfs,
	AC_HELP_STRING([--disable-tftpfs],
		[disable use of TFTPFS])
)

AC_ARG_ENABLE(rsh-symtab,
	AC_HELP_STRING([--disable-rsh-symtab],
		[disable support for downloading symbol table or startup script via RSH])
)

AC_ARG_ENABLE(opcodes,
	AC_HELP_STRING([--disable-opcodes],
		[disable the use of the opcodes library (if present)])
)

AC_ARG_ENABLE(unified-workarea,
	AC_HELP_STRING([--enable-unified-workarea],
	[enable unified malloc-heap/rtems workspace])
)

AC_ARG_WITH(bfdlib,
	AC_HELP_STRING([--with-bfdlib=gnubfd | pmbfd | pmelf | none],
		[which BFD libraries to use if gesys is built outside of 
	     ssrlApps (ignored otherwise). Must match CEXP configuration;
		 valid values are 'gnubfd', 'pmbfd', 'pmelf' or 'none']),
	[case $withval in
		gnubfd | pmbfd | pmelf | none)
		;;
		*)
		AC_MSG_ERROR([invalid value for --with-bfdlib option])
	 esac]
)

AC_ARG_WITH(nic-attach,
	AC_HELP_STRING([--with-nic-attach=<rtems_bsdnet_nic_attach_function_name>],
		[which NIC attach function to use (if not or not 
	     correctly defined by BSP). You will want to use
		 --with-nic-name also; see below.
		(As a special case, 'multi' is allowed which
		links a few drivers and probes them at run-time.)])
)
	
AC_ARG_WITH(nic-name,
	AC_HELP_STRING([--with-nic-name=<rtems_bsdnet_nic_name_string>],
		[which NIC device name to use (if not or not 
	     correctly defined by BSP). You will want to use
		 --with-nic-attach also; see above. This must
		 be a (properly quoted) C-style string constant.])
)

AC_ARG_WITH(extra-gesys-libs,
	AC_HELP_STRING([--with-extra-gesys-libs="-lmylib -lmyotherlib"],
		[List additional libraries you want to build into GeSys.
		 If they are installed outside of the RTEMS tree then
		 you want to use --with-extra-libdirs, too.])
)


AH_TEMPLATE([HAVE_LIBNETBOOT])
AH_TEMPLATE([HAVE_TECLA])
AH_TEMPLATE([WINS_LINE_DISC])
AH_TEMPLATE([NFS_SUPPORT])
AH_TEMPLATE([TFTP_SUPPORT])
AH_TEMPLATE([RSH_SUPPORT])
AH_TEMPLATE([HAVE_PCIBIOS])

NFSLIB=
if test ! "$enable_nfs" = "no"; then
# looking for bundled NFS;
# Note the 4th argument; linking would fail because
# the application usually supplies rtems_bsdnet_config.
# In order to link, we create a dummy symbol.
enable_nfs=no
AC_CHECK_LIB([nfs],[nfsInit],
	[AC_MSG_NOTICE([Using RTEMS bundled NFS])
	 NFSLIB="$NFSLIB -lnfs"
	 enable_nfs=bundled],
	[if SSRLAPPS_CHECK_COMPONENT(rtemsNfs) ; then
	    AC_MSG_NOTICE([Using unbundled NFS])
		NFSLIB="$NFSLIB -lrtemsNfs -lnfsprot"
		enable_nfs=unbundled
	else
		AC_MSG_NOTICE([No NFS found; disabling support])
	fi],
	TILLAC_RTEMS_CHECK_LIB_ARGS)
fi

if test ! "$enable_nfs" = "no"; then
AC_DEFINE([NFS_SUPPORT],1,[Whether to use NFS])
AC_CHECK_DECL([rpcUdpSeedXidUpper],
	[AC_DEFINE([RPCIO_HAS_SEED_XID_UPPER],1,[Whether RPCIOD supports initializing XID])],
	,
	[#include <rtems.h>
	#include <librtemsNfs.h>])
fi

if test ! "$SUPERPACKAGE_VERSION" = "$PACKAGE_VERSION" ; then
AC_DEFINE_UNQUOTED([SUPERPACKAGE_VERSION],"${SUPERPACKAGE_VERSION}",[Version of a superpackage that Gesys is part of])
fi

if test ! "$SUPERPACKAGE_NAME" = "$PACKAGE_NAME" ; then
AC_DEFINE_UNQUOTED([SUPERPACKAGE_NAME],"${SUPERPACKAGE_NAME}",[Name of a superpackage that Gesys is part of])
fi

if test ! "$enable_tftpfs" = "no" ; then
AC_DEFINE([TFTP_SUPPORT],1,[Whether to build-in support for the TFTP file system])
fi

if test ! "$enable_rsh_symtab" = "no" ; then
AC_DEFINE([RSH_SUPPORT],1,[Whether to build-in support for loading a symbol table via RSH])
fi

AC_MSG_CHECKING([for 'dirutils'])
if test "$enable_nfs" = "unbundled" ; then
	AC_MSG_RESULT([OK; using 'dirutils' from $srcdir/rtemsNfs/src])
	GESYSLIBS="$GESYSLIBS -ldirutils"
else
	AC_CHECK_LIB([dirutils],[rm],
		[AC_MSG_RESULT([Using existing/installed 'dirutils' library])
		 GESYSLIBS="$GESYSLIBS -ldirutils"],
		[if SSRLAPPS_CHECK_COMPONENT(rtemsNfs) ; then
			AC_MSG_RESULT([Using 'dirutils' from unbundled rtemsNfs])
			GESYSLIBS="$GESYSLIBS -ldirutils"
		else
			AC_MSG_RESULT([NO. Not using 'dirutils'])
		fi],
		TILLAC_RTEMS_CHECK_LIB_ARGS)
fi

AC_MSG_CHECKING([if 'libtecla' can be used in this configuration])
if  test ! "${enable_tecla}" = "no" ; then
	enable_tecla=yes
	AC_CHECK_LIB([tecla_r],[gl_get_line],
		[AC_MSG_RESULT([YES - using existing/installed TECLA library])],
		[if SSRLAPPS_CHECK_COMPONENT(cexp) ; then
			AC_MSG_RESULT([YES - using TECLA from ssrlApps/cexp])
		 else
		    enable_tecla=no
		 	AC_MSG_RESULT([NO - no TECLA found!])
		 fi],
		TILLAC_RTEMS_CHECK_LIB_ARGS)
else
	AC_MSG_RESULT([NO - use of TECLA disabled via --disable-tecla])
fi

if test "${enable_tecla}" = "yes" ; then
	AC_DEFINE([HAVE_TECLA],1,[If we should use the tecla line editor])
	AC_DEFINE([WINS_LINE_DISC],1,[Use/install a special line-discipline to find terminal window size])
fi

if  test ! "${enable_netboot}" = "no" ; then
	AC_MSG_CHECKING([if 'netboot' is supported by BSP $enable_rtemsbsp])
	case "$enable_rtemsbsp" in
		svgm|beatnik|uC5282|mvme3100)
			AC_MSG_RESULT([Yes])
			enable_netboot=yes
		;;
		*)
			AC_MSG_RESULT([No; disabling build for this BSP])
			enable_netboot=no
		;;
	esac
	if test "$enable_netboot" = yes ; then
		if test ! "$enable_tecla" = yes ; then
			AC_MSG_NOTICE([No TECLA support; won't use libnetboot])
			enable_netboot=no
		fi
		if test "$enable_nfs" = no ; then
			AC_MSG_NOTICE([No NFS support; won't use libnetboot])
			enable_netboot=no
		fi
		if test "$enable_netboot" = "yes" ; then
			AC_CHECK_LIB([netboot],[bootConfigShow],
				[],
				[if SSRLAPPS_CHECK_COMPONENT(netboot) ; then
					true
				else
					enable_netboot=no
				fi],
				[TILLAC_RTEMS_CHECK_LIB_ARGS -ltecla])
		fi
	fi
fi

if test "${enable_netboot}" = "yes" ; then
	AC_DEFINE([HAVE_LIBNETBOOT],1,[If we should use the netboot library])
fi

have_libbspext=no
AC_MSG_CHECKING([for libbspExt])
AC_CHECK_LIB([bspExt],[bspExtInit],
	[AC_MSG_RESULT([Yes])
	 have_libbspext=yes],
	[if SSRLAPPS_CHECK_COMPONENT(libbspExt) ; then
	 	AC_MSG_RESULT([Yes (part of ssrlApps build)])
	 	have_libbspext=yes
	 else
	 	AC_MSG_RESULT([No])
	 fi],
	TILLAC_RTEMS_CHECK_LIB_ARGS)

if test "$have_libbspext" = "yes" ; then
	AC_DEFINE([HAVE_LIBBSPEXT],1,[if we should use the bsp extension library])
fi

# check for <pcibios.h>
AC_CHECK_HEADER(
	[pcibios.h],
	AC_DEFINE([HAVE_PCIBIOS],[1],[whether we have <pcibios.h> and need to initialize]),
	AC_MSG_NOTICE([<pcibios.h> not found])
)

# icmpping is part of miscUtils -- but not a library we actually link (yet)
#AC_CHECK_HEADER(
#	[icmpping.h],
#	AC_DEFINE([HAVE_ICMPPING_H],[1],[whether we have <icmpping.h>]),
#	AC_MSG_NOTICE([<icmpping.h> not found])
#)

AC_CHECK_DECL([BSP_runtime_console_select],
	[pcx86_console_sel=yes],
	,
	[#include <rtems.h>
	#include <bsp.h>])

# Several BSP-specific settings
case "$rtems_bsp" in
	beatnik | mvme3100 | pc?86* | qemuprep*)
		MEMORY_CONF="'-DMEMORY_HUGE'"
	;;
	mvme167)
		MEMORY_CONF="'-DMEMORY_SCARCE=(1024*1024)'"
	;;
	uC5282)
		MEMORY_CONF="'-DMEMORY_SCARCE=(2*1024*1024)'"
	;;
	*)
	;;
esac

CEXP_TEXT_REGION_END=
RTEMS_CEXP_TEXT_REGION_SIZE=
case "$rtems_bsp" in
	beatnik | mvme3100 | virtex | rce405 | psim | qemuprep* )
		RTEMS_CEXP_TEXT_REGION_SIZE=0x01000000
	;;
	*)
	;;
esac

if test ! "${RTEMS_CEXP_TEXT_REGION_SIZE}"xx = "xx" ; then
	AC_DEFINE_UNQUOTED([RTEMS_CEXP_TEXT_REGION_SIZE],[$RTEMS_CEXP_TEXT_REGION_SIZE],[The amount of memory in the first 32MB area to set aside for loadable module's text (PPC only)])
fi

# BSP_commandline_string is not a function but
# the macro just checks if it can link the symbol
# which is good enough
AC_CHECK_FUNCS([BSP_commandline_string])
AC_CHECK_DECL([BSP_commandline_string],
	[AC_DEFINE([DECL_BSP_COMMANDLINE_STRING],1,[Whether BSP_commandline_string is declared in <bsp.h>])],,
	[#include <rtems.h>
	#include <bsp.h>])

AC_CHECK_FUNCS([rtems_bsdnet_loopattach])

AC_CHECK_FUNCS([bsp_cmdline])

if test "$ac_cv_func_BSP_commandline_string" = "yes" ; then
	EARLY_CMDLINE_GET_TYPE=BSP_STR
elif test "$ac_cv_func_bsp_cmdline" = "yes" ; then
	EARLY_CMDLINE_GET_TYPE=BSP_CLN
else
	case "$rtems_bsp" in
	uC5282)
		EARLY_CMDLINE_GET_TYPE=INTERNAL_NB
	;;
	*)
	;;
	esac
fi

# Check if we can set the Cexp prompt to the hostname
if SSRLAPPS_CHECK_COMPONENT(cexp) ; then
	# assume it's a current version
	AC_MSG_NOTICE([Assuming we may use cexpSetPrompt() because cexp is part of this package...])
	AC_DEFINE([HAVE_CEXP_SET_PROMPT],1,[Whether cexpSetPrompt is available from <cexp.h>])
else
	AC_CHECK_DECL([cexpSetPrompt],
		[AC_DEFINE([HAVE_CEXP_SET_PROMPT],1,[Whether cexpSetPrompt is available from <cexp.h>])],,
		[#include <rtems.h>
#include <cexp.h>])
fi

case "$rtems_bsp" in
	pc?86*)
		if test "${with_nic_name}xx" = "xx" && test "${with_nic_attach}xx" = "xx"  ; then
			with_nic_attach=libbsdport_netdriver_attach
		fi
	;;
	mvme2307|mvme2100)
		if test "${with_nic_name}xx" = "xx" -a "${with_nic_attach}xx" = "xx"  ; then
			with_nic_name='"dc1"'
			with_nic_attach=rtems_dec21140_driver_attach
		fi
	;;
esac

if test xx"${with_nic_attach}" = xxlibbsdport_netdriver_attach ; then
	AC_MSG_CHECKING([for 'libbsdport' and extra PCI ethernet drivers])
	AC_CHECK_LIB([bsdport],[libbsdport_netdriver_attach],
		[AC_MSG_RESULT([Yes])
		 have_libbsdport=yes],
		[if SSRLAPPS_CHECK_COMPONENT(bsd_eth_drivers) ; then
		    AC_MSG_RESULT([Yes (part of ssrlApps build)])
		    have_libbsdport=yes
		 else
		    AC_MSG_RESULT([No])
		 fi],
		TILLAC_RTEMS_CHECK_LIB_ARGS)
	if test "${have_libbsdport}" = "yes" ; then
		if test "${with_nic_name+set}" = "set" ; then
			NIC_LIBS="-lif_`echo ${with_nic_name} | sed -e 's/[[\"0-9]]*//g'` -lbsdport"
		else
		    NIC_LIBS="-lif_pcn -lif_em -lif_le -lif_fxp -lif_re -lbsdport"
		    with_nic_name='""'
		fi
	else
		with_nic_attach=
		with_nic_name=
	fi
fi

AC_MSG_NOTICE([with_nic_name now $with_nic_name])
AC_MSG_NOTICE([with_nic_attach now $with_nic_attach])

if test ! "${with_nic_attach}xx" = "xx" ; then
	case "${with_nic_attach}" in
		multi)
			AC_DEFINE([MULTI_NETDRIVER],1,[Link a few NIC drivers and probe at run-time])
			with_nic_name=
		;;
		* )
			AC_DEFINE_UNQUOTED([NIC_ATTACH],[$with_nic_attach],[NIC attach function we should use])
		;;
	esac
fi

if test ! "${with_nic_name}xx" = "xx" ; then
	AC_DEFINE_UNQUOTED([NIC_NAME],[$with_nic_name],[NIC device name we should use])
fi

if test -d ../cexp/binutils ; then
	if ! test "${enable_opcodes+set}" = "set" ; then
		AC_MSG_NOTICE(['binutils' directory found relative to 'system'; using opcodes library (configure with --disable-opcodes to disable)])
		enable_opcodes=yes
	fi
fi

AC_MSG_CHECKING([which BFD library to use])
if SSRLAPPS_CHECK_COMPONENT(cexp) ; then
	if test -d ../cexp/binutils -a ! "${enable_bfd}" = "no" ; then
		with_bfdlib=gnubfd
	elif test "$enable_loader" = "no" ; then
		if test "$enable_elfsyms" = "no" ; then
			with_bfdlib=none
		else
	    	with_bfdlib=pmelf
		fi
	else
	    with_bfdlib=pmbfd
	fi
else
	if ! test "${with_bfdlib+set}" = "set" ; then
		if test "$enable_loader" = "no" ; then
			if test "$enable_elfsyms" = "no" ; then
				with_bfdlib=none
			else
				with_bfdlib=pmelf
			fi
		else
			AC_MSG_WARN([It seems that you build GeSys apart from CEXP; I must guess which BFD library (pmbfd, no opcodes) to use -- provide --with-bfdlib to help me])
			with_bfdlib=pmbfd
		fi
	fi
fi
AC_MSG_RESULT([$with_bfdlib])

AC_MSG_CHECKING([whether to reduce the system clock rate for this BSP])
case "$rtems_bsp" in
	mvme167|uC5282|virtex|rce405)
		us_per_clock_tick=10000;
	;;
	*)
	;;
esac

if test "$us_per_clock_tick"x = "x" ; then
	us_per_clock_tick="NO"
else
	AC_DEFINE_UNQUOTED([CONFIGURE_MICROSECONDS_PER_TICK],[$us_per_clock_tick],[Override system clock rate defined in 'config.c'])
	us_per_clock_tick="YES: $us_per_clock_tick uS"
fi

AC_MSG_RESULT([$us_per_clock_tick])

if test x"$enable_unified_workarea" = x"yes" ; then
	AC_DEFINE([USE_UNIFIED_WORKAREA], 1, [Whether to use unified area for malloc heap and RTEMS workspace])
fi


package_incpath=
package_libpath=
if test "$tillac_rtems_inside_package" = "yes" ; then
package_incpath=-I'$(top_builddir)/../data/include'
package_libpath=-L'$(top_builddir)/../data/lib'
fi

AC_SUBST([package_incpath])
AC_SUBST([package_libpath])

AC_SUBST([MEMORY_CONF])

AC_SUBST([NFSLIB])
AC_SUBST([GESYSLIBS],[$GESYSLIBS $with_extra_gesys_libs])
AC_SUBST([RTEMS_BSP],[$rtems_bsp])
AC_SUBST([EARLY_CMDLINE_GET_TYPE])
AC_SUBST([NIC_LIBS])
AC_SUBST([CEXP_TEXT_REGION_END])

AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(myspec)

AM_CONDITIONAL([NETBOOT],[test "$enable_netboot"  = "yes"])
AM_CONDITIONAL([TECLA],  [test "$enable_tecla"    = "yes"])
AM_CONDITIONAL([BSPEXT], [test "$have_libbspext"  = "yes"])
AM_CONDITIONAL([OPCODES],[test "$enable_opcodes"  = "yes"])

AM_CONDITIONAL([GNUBFD], [test "$with_bfdlib"     = "gnubfd"])
AM_CONDITIONAL([PMBFD],  [test "$with_bfdlib"     = "pmbfd"])
AM_CONDITIONAL([PMELF],  [test "$with_bfdlib"     = "pmelf"])

# two ways for handling this - the RESERVE_CEXP_TEXT_REGION way is
# an ugly hack; we really should have proper linker scripts...
AM_CONDITIONAL([RESERVE_CEXP_TEXT_REGION],  [test ! "$CEXP_TEXT_REGION_END"xx = "xx" ])
AM_CONDITIONAL([INTERNAL_CEXP_TEXT_REGION], [test ! "$RTEMS_CEXP_TEXT_REGION_SIZE"xx = "xx" ])

AM_CONDITIONAL([CONSOLE_SEL], [test "$pcx86_console_sel" = "yes"])

TILLAC_RTEMS_BSP_POSTLINK_CMDS

AC_OUTPUT
