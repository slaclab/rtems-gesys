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
C_PIECES=init rtems_netconfig config
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

PGMS=${ARCH}/rtems.exe

# List of RTEMS managers to be included in the application goes here.
# Use:
#     MANAGERS=all
# to include all RTEMS managers in the application.
MANAGERS=all

include $(RTEMS_MAKEFILE_PATH)/Makefile.inc

include $(RTEMS_CUSTOM)
include $(RTEMS_ROOT)/make/leaf.cfg

#
# (OPTIONAL) Add local stuff here using +=
#

DEFINES  +=

CPPFLAGS += -I. -I/afs/slac/package/rtems/prod/rtems/powerpc-rtems/include 
CFLAGS   += -O2

#
# CFLAGS_DEBUG_V are used when the `make debug' target is built.
# To link your application with the non-optimized RTEMS routines,
# uncomment the following line:
# CFLAGS_DEBUG_V += -qrtems_debug
#

LD_PATHS  += /home/till/slac/cexp/build-ppc-rtems /afs/slac/package/rtems/prod/rtems/powerpc-rtems/lib /home/till/rtems/apps/libtecla/build-ppc-rtems
#LD_LIBS   += -Wl,--whole-archive -lcexp -lbfd -lregexp -lopcodes -liberty -lrtemscpu -lrtemsbsp  -lc
LD_LIBS   += -lcexp -lbfd -lregexp -lopcodes -liberty -ltecla_r -Wl,-T,symlist.lds  -lm
LDFLAGS   += -Wl,-u,inet_pton

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

$(filter %.exe,$(PGMS)): ${OBJS} ${LINK_FILES}
	$(make-exe)
	xsyms $@ $(@:%.exe=%.sym)

# Install the program(s), appending _g or _p as appropriate.
# for include files, just use $(INSTALL_CHANGE)
install:  all
	$(INSTALL_VARIANT) -m 555 ${PGMS} ${PROJECT_RELEASE}/bin
