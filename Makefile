#
#  $Id$
#
#

# On a PC console (i.e. VGA, not a serial terminal console)
# tecla should _not_ be used because the RTEMS VGA console
# driver is AFAIK non-ANSI and too dumb to be used by tecla.
# Don't forget to configure Cexp with --disable-tecla in this
# case...
# UPDATE: I just filed PR#502 to RTEMS-4.6.0pre4 - if you
#         have this, libtecla should work fine (2003/9/26)
# On a PC, you may have to use a different network driver
# also; YMMV. The value must be 'YES' or 'NO' (no quotes) 
USE_TECLA  = YES
# Whether to use the libbspExt library. This is always
# (automagically) disabled on pcx86 BSPs.
USE_BSPEXT = YES

# Include NFS support; system symbol table and initialization
# scripts can be loaded using NFS
USE_NFS    = YES

# Include TFTP filesystem support; system symbol table and
# initialization scripts can be loaded using TFTP
USE_TFTPFS = YES

# Include RSH support for downloading the system symbol
# table and a system initialization script (user level
# script not supported).
# NOTE: RSH support is NOT a filesystem but just downloads
#       the essential files (symfile and script) to the IMFS,
#       i.e., NO path is available to the script.
USE_RSH    = YES

#

# Optional libraries to add, i.e. functionality you
# want to be present but which is not directly used
# by GeSys itself.
#
# By default, GeSys links in as much as it can from
# any library it knows of (libc, librtemscpu, OPT_LIBRARIES
# etc. you can list objects you want to exclude explicitely
# in the EXCLUDE_LISTS below.
OPT_LIBRARIES = -lm -lrtems++

# Particular objects to EXCLUDE from the link.
# '$(NM) -fposix -g' generated files are acceptable
# here. You can e.g. copy $(ARCH)/librtemscpu.nm 
# (generated during a first run) to librtemscpu.excl
# and edit it to only contain stuff you DONT want.
# Then mention it here
# 
EXCLUDE_LISTS+=$(wildcard config/*.excl)

# This was permantently excluded from librtemsbsp.a by a patch
#EXCLUDE_LISTS+=config/libnetapps.excl  

#
# Particular objects to INCLUDE with the link (note that
# dependencies are also added).
#
# Note: INCLUDE_LISTS override EXCLUDE_LISTS; if you need more
#       fine grained control, you can pass include/exclude list
#       files explicitely  to 'ldep' and they are processed
#       in the order they appear on the command line.
INCLUDE_LISTS+=$(wildcard config/*.incl)

# RTEMS versions older than 4.6.0 (and probably newer than 4.5)
# have a subtle bug (PR504). Setting to YES installs a fix for this.
# This should be NO on 4.6.0 and later.
USE_GC=NO


# C source names, if any, go here -- minus the .c
# make a system for storing in flash. The compressed binary
# can be generated with the tools and code from 'netboot'.
# Note that this is currently limited to compressed images < 512k
#
#C_PIECES=flashInit rtems_netconfig config flash
#

# Normal (i.e. non-flash) system which can be net-booted
USE_TECLA_YES_C_PIECES = term
C_PIECES=init rtems_netconfig config $(USE_TECLA_$(USE_TECLA)_C_PIECES)

# SSRL 4.6.0pre2 compatibility workaround. Obsolete.
#C_PIECES+=pre2-compat

C_FILES=$(C_PIECES:%=%.c)
C_O_FILES=$(C_PIECES:%=${ARCH}/%.o)

# C++ source names, if any, go here -- minus the .cc
CC_PIECES=

CC_PIECES_GC_YES=gc
CC_PIECES+=$(CC_PIECES_GC_$(USE_GC))

CC_FILES=$(CC_PIECES:%=%.cc)
CC_O_FILES=$(CC_PIECES:%=${ARCH}/%.o)

H_FILES=

# Assembly source names, if any, go here -- minus the .S
S_PIECES=
S_FILES=$(S_PIECES:%=%.S)
S_O_FILES=$(S_FILES:%.S=${ARCH}/%.o)

SRCS=$(C_FILES) $(CC_FILES) $(H_FILES) $(S_FILES)
OBJS=$(C_O_FILES) $(CC_O_FILES) $(S_O_FILES)

PGMS=${ARCH}/rtems.exe st.sys

# List of RTEMS managers to be included in the application goes here.
# Use:
#     MANAGERS=all
# to include all RTEMS managers in the application.
MANAGERS=all


include $(RTEMS_MAKEFILE_PATH)/Makefile.inc
include $(RTEMS_CUSTOM)
include $(RTEMS_ROOT)/make/leaf.cfg

ifndef XSYMS
XSYMS = xsyms
endif

#
# (OPTIONAL) Add local stuff here using +=
#
# make-exe uses LINK.c but we have C++ stuff
LINK.c = $(LINK.cc)

DEFINES  += -DUSE_POSIX

# Trim BSP specific things
ifeq  "$(RTEMS_BSP_FAMILY)" "svgm" 
DEFINES  += -DHAVE_BSP_EXCEPTION_EXTENSION
C_PIECES += nvram
ifndef ELFEXT
ELFEXT    = exe
endif 
endif 

ifeq "$(RTEMS_BSP_FAMILY)" "motorola_powerpc"
DEFINES  += -DRTEMS_BSP_NETWORK_DRIVER_NAME=\"dc1\"
DEFINES  += -DRTEMS_BSP_NETWORK_DRIVER_ATTACH=rtems_dec21140_driver_attach
ifndef ELFEXT
ELFEXT    = nxe
endif
endif

ifeq  "$(RTEMS_BSP_FAMILY)" "pc386"
DEFINES  += -DRTEMS_BSP_NETWORK_DRIVER_NAME=\"fxp1\"
DEFINES  += -DRTEMS_BSP_NETWORK_DRIVER_ATTACH=rtems_fxp_attach
ifndef ELFEXT
ELFEXT    = obj
endif
USE_BSPEXT = NO
endif

ifeq "$(RTEMS_BSP_FAMILY)" "mvme167"
USE_BSPEXT = NO
endif


bspfail:
	$(error GeSys has not been ported/tested on this BSP ($(RTEMS_BSP)) yet)

bspcheck: $(if $(filter $(RTEMS_BSP_FAMILY),pc386 motorola_powerpc svgm mvme167),,bspfail)


CPPFLAGS += -I.
CFLAGS   += -O2
# Enable the stack checker. Unfortunately, this must be
# a command line option because some pieces are built into
# the system configuration table...
#CFLAGS   +=-DSTACK_CHECKER_ON

USE_TECLA_YES_DEFINES  = -DWINS_LINE_DISC -DUSE_TECLA
USE_NFS_YES_DEFINES    = -DNFS_SUPPORT
USE_TFTPFS_YES_DEFINES = -DTFTP_SUPPORT
USE_RSH_YES_DEFINES    = -DRSH_SUPPORT

DEFINES+=$(USE_TECLA_$(USE_TECLA)_DEFINES)
DEFINES+=$(USE_NFS_$(USE_NFS)_DEFINES)
DEFINES+=$(USE_TFTPFS_$(USE_TFTPFS)_DEFINES)
DEFINES+=$(USE_RSH_$(USE_RSH)_DEFINES)

#
# CFLAGS_DEBUG_V are used when the `make debug' target is built.
# To link your application with the non-optimized RTEMS routines,
# uncomment the following line:
# CFLAGS_DEBUG_V += -qrtems_debug
#

USE_TECLA_YES_LIB  = -ltecla_r
USE_BSPEXT_YES_LIB = -lbspExt
USE_NFS_YES_LIB    = -lrtemsNfs

LD_LIBS   += -lcexp -lbfd -lspencer_regexp -lopcodes -liberty
LD_LIBS   += $(USE_TECLA_$(USE_TECLA)_LIB)
LD_LIBS   += $(USE_BSPEXT_$(USE_BSPEXT)_LIB)
LD_LIBS   += $(USE_NFS_$(USE_NFS)_LIB)
LD_LIBS   += $(OPT_LIBRARIES)

# Produce a linker map to help finding 'undefined symbol' references (README.config)
LDFLAGS_GC_YES = -Wl,--wrap,free
LDFLAGS   += -Wl,-Map,$(ARCH)/linkmap $(LDFLAGS_GC_$(USE_GC))

# this special object contains 'undefined' references for
# symbols we want to forcibly include. It is automatically
# generated. 
OBJS      += ${ARCH}/allsyms.o

#
# Add your list of files to delete here.  The config files
#  already know how to delete some stuff, so you may want
#  to just run 'make clean' first to see what gets missed.
#  'make clobber' already includes 'make clean'
#

CLEAN_ADDITIONS   += builddate.c nvram.c
CLOBBER_ADDITIONS +=

all: bspcheck gc-check libnms ${ARCH} $(SRCS) $(PGMS)

# We want to have the build date compiled in...
$(ARCH)/init.o: builddate.c pathcheck.c

builddate.c: $(filter-out $(ARCH)/init.o $(ARCH)/allsyms.o,$(OBJS)) Makefile
	echo 'static char *system_build_date="'`date +%Y%m%d%Z%T`'";' > builddate.c

nvram.c: nvram/nvram.c
	ln -s $^ $@

pathcheck.c: nvram/pathcheck.c
	ln -s $^ $@

# Build the executable and a symbol table file
$(filter %.exe,$(PGMS)): ${LINK_FILES}
	$(make-exe)
ifdef ELFEXT
ifdef XSYMS
ifeq ($(USE_GC),YES)
	$(OBJCOPY) --redefine-sym free=__real_free --redefine-sym __wrap_free=free $(@:%.exe=%.$(ELFEXT))
endif
	$(XSYMS) $(@:%.exe=%.$(ELFEXT)) $(@:%.exe=%.sym)
endif
endif

# Installation
ifndef RTEMS_SITE_INSTALLDIR
RTEMS_SITE_INSTALLDIR = $(PROJECT_RELEASE)
endif

$(RTEMS_SITE_INSTALLDIR)/$(RTEMS_BSP)/bin:
	test -d $@ || mkdir -p $@

INSTFILES = ${PGMS} ${PGMS:%.exe=%.bin} ${PGMS:%.exe=%.sym}

# How to build a  tarball of this package
REVISION=$(filter-out $$%,$$Name$$)
tar:
	@$(make-tar)

# Install the program(s), appending _g or _p as appropriate.
# for include files, just use $(INSTALL_CHANGE)
install: all $(RTEMS_SITE_INSTALLDIR)/$(RTEMS_BSP)/bin
	for feil in $(INSTFILES); do if [ -r $$feil ] ; then  \
		$(INSTALL_VARIANT) -m 555 $$feil ${RTEMS_SITE_INSTALLDIR}/$(RTEMS_BSP)/bin ; fi ; done


# Below here, magic follows for generating the
# 'allsyms.o' object
#

# Our tool; should actually already be defined to
# point to a site specific installated version.
ifndef LDEP
LDEP = ldep/ldep
# emergency build
ldep/ldep: ldep/ldep.c
	cc -O -o $@ $<
CLEAN_ADDITIONS+=ldep/ldep
endif


# Produce an object with undefined references.
# These are listed in the linker script generated
# by LDEP.
SYMLIST_LDS = $(ARCH)/ldep.lds
# We just need an empty object to keep the linker happy

$(ARCH)/allsyms.o:	$(SYMLIST_LDS) $(ARCH)/empty.o
	$(LD) -T$< -r -o $@ $(ARCH)/empty.o

# dummy up an empty object file
$(ARCH)/empty.o:
	touch $(@:%.o=%.c)
	$(CC) -c -O -o $@ $(@:%.o=%.c)

# try to find out what startfiles will be linked in
$(ARCH)/gcc-startfiles.o: $(ARCH)/empty.o
	$(LINK.cc) -Wl,-r -nodefaultlibs -o $@ $^

# and generate a name file for them (the endfiles will
# actually be there also)
$(ARCH)/startfiles.nm: $(ARCH)/gcc-startfiles.o
	$(NM) -g -fposix $^ > $@

# generate a name file for the application's objects
$(ARCH)/app.nm: $(filter-out $(ARCH)/allsyms.o,$(OBJS))
	$(NM) -g -fposix $^ > $@

# NOTE: must not make 'myspec'! Otherwise, the first
#       'make' won't find it when interpreting the makefile.
#
#myspec:
#	$(RM) $@
#	echo '*linker:' >$@
#	echo "`pwd`/mylink" >>$@
#

THELIBS:=$(shell $(LINK.cc) $(AM_CFLAGS) $(AM_LDFLAGS) $(LINK_LIBS) -specs=myspec)

LIBNMS=$(patsubst %.a,$(ARCH)/%.nm,$(sort $(patsubst -l%,lib%.a,$(filter -l%,$(THELIBS)))))
OPTIONAL_ALL=$(addprefix -o,$(LIBNMS)) 
#OPTIONAL_ALL=-o$(ARCH)/app.nm

$(SYMLIST_LDS): $(ARCH)/app.nm $(LIBNMS) $(ARCH)/startfiles.nm $(EXCLUDE_LISTS) $(LDEP)
	echo $^
	$(LDEP) -F -l -u $(OPTIONAL_ALL) $(addprefix -x,$(EXCLUDE_LISTS)) $(addprefix -o,$(INCLUDE_LISTS)) -e $@ $(filter %.nm,$^) > $(ARCH)/ldep.log

vpath %.a $(patsubst -L%,%,$(filter -L%,$(THELIBS)))

libnms: $(ARCH) $(LIBNMS)
	
$(ARCH)/%.nm: %.a
	$(NM) -g -fposix $^ > $@



foo:
	echo $(filter %.nm,$(LIBNMS))

thelibs:
	echo $(THELIBS)


# Create the name files for our libraries. This is achieved by
# invoking the compiler with a special 'spec' file which instructs
# it to use a dummy "linker" ('mylink' shell script). 'mylink'
# then extracts '-L' and '-l' linker command line options to
# localize all libraries we use.
# Finally, we recursively invoke this Makefile again, now
# knowing the libraries.
#libnms: $(ARCH)
#	sh -c "$(MAKE) `$(LINK.cc) $(AM_CFLAGS) $(AM_LDFLAGS) $(LINK_LIBS) -specs=myspec` libnms-recurse"

# 4.6. pre versions require the GC workaround. 4.5 probably not, but we
# don't bother...
gc-check: librtemscpu.a
ifeq ($(USE_GC),YES)
	@if nm $^ | grep -q RTEMS_Malloc_GC_list ; then \
		echo 'Your RTEMS release has bug #504 apparently fixed; set USE_GC to NO' ;\
		exit 1;\
	fi
else
	@if ! nm $^ | grep -q RTEMS_Malloc_GC_list ; then \
		echo 'Your RTEMS release might have bug #504; set USE_GC to YES' ;\
		exit 1;\
	fi
endif

