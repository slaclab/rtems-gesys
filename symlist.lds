/* THIS CONFIGURATION WAS CREATED FOR
 * RTEMS snapshot ss-20030128 (SLAC/SSRL patches applied)
 * newlib-1.11.1
 * GCC-3.2.1 
 *
 * T. Straumann, 2003/4/9
 */

/* GCC support libraries */
  INCLUDE config/libgcc.scr
  INCLUDE config/libstdc++.scr
  INCLUDE config/libsupc++.scr

/* NEWLIB */
  INCLUDE config/libc.scr
  INCLUDE config/libm.scr

/* RTEMS */

  /* libmisc -- UNUSED */
    /* INCLUDE config/libmisc.scr */

  /* libnetapps -- DONT USE; this doesn't belong into RTEMS core */
    /* INCLUDE config/libnetapps.scr */

  /* libblock & friends -- I dont like it nor need it */
    /* INCLUDE config/libblock.scr */
    /* INCLUDE config/libdosfs.scr */

  /* IMFS ('in memory file system') -- NEEDED */
    INCLUDE config/libimfs.scr

  /* libchip -- don't enforce linkage */
    /* INCLUDE config/libnetchip.scr */
    /* INCLUDE config/librtcio.scr */
    /* INCLUDE config/libserialio.scr */

  /* networking -- NEEDED */
    INCLUDE config/libnetworking.scr
  /* RPC */
    INCLUDE config/librpc.scr
    INCLUDE config/libxdr.scr
  /* RDBG (remote debugging support) -- is broken but I leave it for the moment */
    INCLUDE config/librdbg.scr

  /* RTEMS CORE */
    INCLUDE config/librtems.scr
    INCLUDE config/libsapi.scr
    INCLUDE config/libposix.scr
    INCLUDE config/libscore.scr
    INCLUDE config/libcsupport.scr
    INCLUDE config/librtems++.scr

  /* BSP CORE */
    INCLUDE config/libbsp.scr

/* SITE SPECIFIC LIBRARIES */
  INCLUDE config/libbspExt.scr
  INCLUDE config/libtecla_r.scr
  INCLUDE config/libspencer_regexp.scr
