AUTOMAKE_OPTIONS=foreign

ACLOCAL_AMFLAGS = -I./m4

EXEEXT          = $(APPEXEEXT)

AM_CPPFLAGS     = -Wall -I$(srcdir)/nvram @package_incpath@
AM_CFLAGS       = -Wall
AM_CXXFLAGS     = -Wall
AM_CCASFLAGS    = 
AM_LDFLAGS      = @package_libpath@
DOWNEXT         = @DOWNEXT@

# initialize vars
GESYSLIBS       = @GESYSLIBS@
rtems_LDADD     =
APPOBJS         =

bin_PROGRAMS    = rtems
# must use SCRIPTS, automake doesn't allow DATA into bindir
bin_SCRIPTS     =
if HAVE_BSP_POSTLINK_CMDS
bin_SCRIPTS    += rtems$(DOWNEXT)
endif
bin_SCRIPTS    += st.sys

EXTRA_DIST      = mylink makefile.top.am makefile.top.in myspec bootstrap
EXTRA_DIST     += dummycxx
EXTRA_DIST     += psim_tree.gesys
EXTRA_DIST     += README.addpath README.config
EXTRA_DIST     += LICENSE
EXTRA_DIST     += $(wildcard $(srcdir)/st.sys*)
EXTRA_DIST     += ldep
EXTRA_DIST     += config

rtems_CPPFLAGS  = $(AM_CPPFLAGS)

rtems_SOURCES   = init.c config.c
if HAVE_LIBBSD
AM_LDFLAGS     += -Wl,--defsym,ticks=_Watchdog_Ticks_since_boot
if NEED_LIBBSD_LD_HACKS
AM_LDFLAGS     += -Wl,--defsym,_bind=bind
AM_LDFLAGS     += -Wl,--defsym,_getsockopt=getsockopt
AM_LDFLAGS     += -Wl,--defsym,_pthread_mutex_lock=pthread_mutex_lock
AM_LDFLAGS     += -Wl,--defsym,pthread_main_np=0
endif
rtems_SOURCES  += rtems_libbsdconfig.c
else
rtems_SOURCES  += rtems_netconfig.c
endif
rtems_SOURCES  += addpath.c
if NETBOOT
else
rtems_SOURCES  += nvram/pairxtract.c
endif
if WRAP_EXIDX
rtems_SOURCES  += gnuUnwindExidxWrapper.c
AM_LDFLAGS     += -Wl,--wrap,__gnu_Unwind_Find_exidx
endif

rtems_SOURCES  += nvram/minversion.h verscheck.h

if TECLA
rtems_SOURCES  += term.c
endif
# Need one C++ source because we want automake to use
# the C++ linker (and C++ system libraries)
rtems_SOURCES  += cppdummy.cc

if INTERNAL_CEXP_TEXT_REGION
rtems_SOURCES  += cexp-txtregion.c
endif

EXTRA_rtems_SOURCES=

EXTRA_rtems_SOURCES    += bug_disk.c bev.c reboot5282.c nvram/pathcheck.c
EXTRA_rtems_SOURCES    += pcx86_console_select.c

# PSIM bsp
psim_bspclass_OBJS      = bug_disk.$(OBJEXT)
psim_bspclass_DEFINES   = -DPSIM

# uC5282 bsp
uC5282_bspclass_OBJS    = bev.$(OBJEXT) reboot5282.$(OBJEXT)
if NETBOOT
uC5282_bspclass_DEFINES =
else
uC5282_bspclass_DEFINES =-DBSP_NETWORK_SETUP=bev_network_setup
endif

rtems_CPPFLAGS      += -DUSE_POSIX

if CONSOLE_SEL
APPOBJS             += pcx86_console_select.$(OBJEXT)
endif

if NETBOOT
EARLY_CMDLINE_GET_INTERNAL_NB='-DEARLY_CMDLINE_GET(arg)=do { *(arg) = 0; /* use internal buffer */ } while (0)'
endif
EARLY_CMDLINE_GET_INTERNAL='-DEARLY_CMDLINE_GET(arg)=do { *(arg) = 0; /* use internal buffer */ } while (0)'
EARLY_CMDLINE_GET_BSP_STR='-DEARLY_CMDLINE_GET(arg)=do { *(arg) = BSP_commandline_string; } while (0)'
EARLY_CMDLINE_GET_BSP_CLN='-DEARLY_CMDLINE_GET(arg)=do { *(arg) = bsp_cmdline(); } while (0)'

# Add BSP-specific defines + objects
APPOBJS             += $(@RTEMS_BSP_CLASS@_bspclass_OBJS)
rtems_CPPFLAGS      += $(@RTEMS_BSP_CLASS@_bspclass_DEFINES)

rtems_CPPFLAGS      += @MEMORY_CONF@
rtems_CPPFLAGS      += $(EARLY_CMDLINE_GET_@EARLY_CMDLINE_GET_TYPE@)

# Add automatically generated symbol table
rtems_LDADD    += allsyms.$(OBJEXT) objattrs.$(OBJEXT)
GESYSLIBS      += -lcexp 

if OPCODES
GESYSLIBS      += -lopcodes
endif
if GNUBFD
GESYSLIBS      += -lbfd -liberty
else
if PMBFD      
if OPCODES
GESYSLIBS      += -liberty
endif
GESYSLIBS      += -lpmbfd -lpmelf
else
if PMELF
GESYSLIBS      += -lpmelf
endif
endif
endif

if NETBOOT
GESYSLIBS      += -lnetboot
endif
if TECLA
GESYSLIBS      += -ltecla_r
endif

GESYSLIBS      += @NFSLIB@

GESYSLIBS      += -lspencer_regexp

GESYSLIBS      += @NIC_LIBS@

if BSPEXT
GESYSLIBS      += -lbspExt
endif

GESYSLIBS      += @BSDLIB@

rtems_LDADD    += $(APPOBJS) $(GESYSLIBS)
rtems_DEPENDENCIES = $(rtems_LDADD)

init.c: builddate.c

CLEANFILES      = $(wildcard *.nm) allsyms.c objattrs.c builddate.c gcc-startfiles$(EXEEXT)
CLEANFILES     += $(bin_SCRIPTS)
DISTCLEANFILES  = ldep.log

# Find out what libraries will be linked (including system libraries).
# This is achieved by invoking the compiler with a special 'spec' file
# which instructs it to use a dummy "linker" ('mylink' shell script).
# 'mylink' then extracts '-L' and '-l' linker command line options to
# localize all libraries we use. The various -Lxxx -lyyy 'options'
# are listed in 'THELIBS'.
# You can invoke the Makefile with the 'dbg-thelibs' target to
# show what THELIBS is set to.
THELIBS:=$(shell $(CXXLINK) $(rtems_LDFLAGS) $(GESYSLIBS) $(LIBS) -specs=myspec)

# Set a vpath for .a files (libraries) so that make knows where
# to look for libraries when we extract their symbol tables.
vpath %.a $(patsubst -L%,%,$(filter -L%,$(THELIBS)))

# try to find out what startfiles will be linked in
# and what symbols are defined by the linker script
gcc-startfiles$(EXEEXT):
	$(CXXLD) -nodefaultlibs -o $@ -Wl,--unresolved-symbols=ignore-all -T`$(CC) -print-file-name=linkcmds`

# and generate a name file for them (the endfiles will
# actually be there also)
# Map '?' to abs symbol in startfile (hack for some linker script
# addresses)
startfiles.nm: gcc-startfiles$(EXEEXT)
	$(NM) -g -fposix $^ | sed -e 's/\([^ ]*\) [?]/\1 A/' > $@

# generate a name file for the application's objects
app.nm: $(filter-out allsyms.$(OBJEXT) objattrs.$(OBJEXT),$(rtems_OBJECTS) $(APPOBJS))
	$(NM) -g -fposix $^ > $@

LIBNMS=$(patsubst %.a,%.nm,$(sort $(patsubst -l%,lib%.a,$(filter -l%,$(THELIBS)))))
OPTIONAL_ALL=$(addprefix -o,$(LIBNMS)) 


# Use 'ldep' to analyze library interdependencies and to 
# generate a symbol table.
allsyms.c: app.nm $(LIBNMS) startfiles.nm $(EXCLUDE_LISTS)
	echo $^
	$(LDEP) -F -l -u $(OPTIONAL_ALL) $(addprefix -x,$(EXCLUDE_LISTS)) $(addprefix -o,$(INCLUDE_LISTS)) -C $@ $(filter %.nm,$^)  > ldep.log

libnms: $(ARCH) $(LIBNMS)
	
# How to produce an (ASCII) symbol table from a library archive
%.nm: %.a
	$(NM) -g -fposix $^ > $@

# Extract BFD 'architecture' from a binary
define extract-arch
	$(OBJDUMP) -f $< | sed -ne 's/^\([ \t]*architecture[:]\{0,1\}[ \t]*\)\([^,]*\)\(.*\)$$/\2/gp'
endef

define extract-objattrs
	@( \
	echo '/*' ;\
	echo ' * DO NOT EDIT THIS FILE -- IT WAS GENERATED AUTOMATICALLY' ;\
	echo ' * (dump of '.gnu.attributes' section)' ;\
	echo ' */' ;\
	echo 'static const unsigned char thebytes[] = {' ;\
	$(READELF) -x .gnu.attributes $< 2>/dev/null | sed \
	-e '/^[ \t]*0x[0-9a-fA-F]\{4,8\} /!d' \
	-e '/^[ \t]*0x[0-9a-fA-F]\{4,8\} /s/^\([ \t]*0x[0-9a-fA-F]\{4,8\} \)\(.\{36\}\)\(.*\)$$/\2/g' \
	-e 's/[0-9a-fA-F]\{2\}/0x&,/g' \
	-e 's/ //g' ;\
	echo '};' ;\
	echo 'const unsigned char *cexpSystemAttributes = thebytes;' ;\
	echo 'const unsigned cexpSystemAttributesSize   = sizeof(thebytes);' \
	)
endef

# Hack linker symbols to reserve memory for cexp linking the text
# region into
REGLDFLAGS=
# define the end of the reserved region; must be <= 32M for code
# to be reachable
REGLDFLAGS+=-Wl,--defsym,_cexpTextRegionEnd=@CEXP_TEXT_REGION_END@

#define new versions for '_end', '__rtems_end' and 'end'
REGLDFLAGS+=-Wl,--defsym,__wrap__end=_cexpTextRegionEnd
REGLDFLAGS+=-Wl,--defsym,__wrap___rtems_end=_cexpTextRegionEnd
REGLDFLAGS+=-Wl,--defsym,__wrap_end=_cexpTextRegionEnd
# wrap the symbols 'end' '_end' '__rtems_end' so they resolve to the
# wrapped variants
REGLDFLAGS+=-Wl,--wrap,_end
REGLDFLAGS+=-Wl,--wrap,__rtems_end
REGLDFLAGS+=-Wl,--wrap,end
# define the start of the reserved region
REGLDFLAGS+=-Wl,--defsym,_cexpTextRegionStart=__real__end

if RESERVE_CEXP_TEXT_REGION
AM_LDFLAGS+=$(REGLDFLAGS)
endif

builddate.c: $(filter-out rtems-init.$(OBJEXT) rtems-allsyms.$(OBJEXT),$(rtems_OBJECTS)) Makefile
	echo 'const char *GeSys_Build_Date="'`date +%Y%m%d%Z%T`'";' > $@
	echo '#define DEFAULT_CPU_ARCH_FOR_CEXP "'`$(extract-arch)`'"' >>$@

objattrs.c: rtems-init.$(OBJEXT)
	$(extract-objattrs) > $@

dbg-libnms:
	echo $(filter %.nm,$(LIBNMS))

dbg-thelibs:
	@echo THELIBS:
	@echo $(THELIBS)
	@echo
	@echo Library vpath:
	@echo $(patsubst -L%,%,$(filter -L%,$(THELIBS)))

if HAVE_BSP_POSTLINK_CMDS
%$(DOWNEXT):%$(EXEEXT)
	$(RTEMS_BSP_POSTLINK_CMDS)
endif

st.sys: st.sys.in $(wildcard $(srcdir)/st.sys-$(RTEMS_BSP))
	cat $^ | sed -e 's/[@]RTEMS_CPU[@]/$(host_cpu)/g' -e 's/[@]RTEMS_BSP[@]/$(RTEMS_BSP)/g' > $@

#include $(top_srcdir)/rtems.am
