#
#  $Id$
#
# Templates/Makefile.leaf
# 	Template leaf node Makefile
#

# C source names, if any, go here -- minus the .c
# make a system for storing in flash. The compressed binary
# can be generated with the tools and code from 'netboot'.
# Note that this is currently limited to compressed images < 512k
#
#C_PIECES=flashInit rtems_netconfig config flash
#
# Normal system which can be net-booted
C_PIECES=init rtems_netconfig config term
C_FILES=$(C_PIECES:%=%.c)
C_O_FILES=$(C_PIECES:%=${ARCH}/%.o)

# C++ source names, if any, go here -- minus the .cc
CC_PIECES=
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
#include f


#
# (OPTIONAL) Add local stuff here using +=
#
LINK.c = $(LINK.cc)

ELFEXT = exe

ifeq  "$(RTEMS_BSP)" "svgm" 
DEFINES  += -DHAVE_BSP_EXCEPTION_EXTENSION
endif
DEFINES  += -DUSE_POSIX

ifeq "$(RTEMS_BSP)" "mvme2307"
DEFINES  += -DRTEMS_BSP_NETWORK_DRIVER_NAME=\"dc1\"
DEFINES  += -DRTEMS_BSP_NETWORK_DRIVER_ATTACH=rtems_dec21140_driver_attach
ELFEXT    = nxe
endif

ifeq "$(RTEMS_BSP)" "pc386"
DEFINES  += -DRTEMS_BSP_NETWORK_DRIVER_NAME=\"fxp1\"
DEFINES  += -DRTEMS_BSP_NETWORK_DRIVER_ATTACH=rtems_fxp_attach
ELFEXT    = obj
endif



CPPFLAGS += -I.
CFLAGS   += -O2

#
# CFLAGS_DEBUG_V are used when the `make debug' target is built.
# To link your application with the non-optimized RTEMS routines,
# uncomment the following line:
# CFLAGS_DEBUG_V += -qrtems_debug
#

#LD_LIBS   += -Wl,--whole-archive -lcexp -lbfd -lspencer_regexp -lopcodes -liberty -lrtemscpu -lrtemsbsp  -lc
LD_LIBS   += -lcexp -lbfd -lspencer_regexp -lopcodes -liberty -ltecla_r -lm -lbspExt
LDFLAGS   += -Wl,--trace-symbol,get_pty -Wl,-Map,map
#LDFLAGS   += -Wl,-T,symlist.lds
#LDFLAGS    += -L$(prefix)/$(RTEMS_CPU)-rtems/lib
OBJS      += ${ARCH}/allsyms.o
#OBJS      += allsyms.o

tst:
	echo $(LINK.c)
	echo $(AM_CFLAGS)
	echo $(AM_LDFLAGS) 
	echo $(LINK_OBJS)
	echo $(LINK_LIBS)
#
# Add your list of files to delete here.  The config files
#  already know how to delete some stuff, so you may want
#  to just run 'make clean' first to see what gets missed.
#  'make clobber' already includes 'make clean'
#

#CLEAN_ADDITIONS += xxx-your-debris-goes-here
CLOBBER_ADDITIONS +=

all:	${ARCH} $(SRCS) $(PGMS)

$(ARCH)/allsyms.o:	symlist.lds $(ARCH)/empty.o config/*
	$(LD) -T$< -r -o $@ $(ARCH)/empty.o

$(ARCH)/empty.o:
	touch $(@:%.o=%.c)
	$(CC) -c -O -o $@ $(@:%.o=%.c)

$(ARCH)/init.o: builddate.c

builddate.c: $(filter-out $(ARCH)/init.o,$(OBJS))
	echo 'static char *system_build_date="'`date +%Y%m%d%Z%T`'";' > $@

$(filter %.exe,$(PGMS)): ${OBJS} ${LINK_FILES}
	$(make-exe)
	xsyms $(@:%.exe=%.$(ELFEXT)) $(@:%.exe=%.sym)

ifndef RTEMS_SITE_INSTALLDIR
RTEMS_SITE_INSTALLDIR = $(PROJECT_RELEASE)
endif

# Install the program(s), appending _g or _p as appropriate.
# for include files, just use $(INSTALL_CHANGE)
install:  all
	$(INSTALL_VARIANT) -m 555 ${PGMS} ${PGMS:%.exe=%.bin} ${PGMS:%.exe=%.sym} ${RTEMS_SITE_INSTALLDIR}/$(RTEMS_BSP)/bin
