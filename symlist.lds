/* THIS CONFIGURATION WAS CREATED FOR
 * RTEMS snapshot ss-20030128 (SLAC/SSRL patches applied)
 * newlib-1.11.1
 * GCC-3.2.1 
 *
 * T. Straumann, 2003/4/9
 */

/* GCC support libraries */
  INCLUDE config.tmpl/libgcc.scr
  INCLUDE config.tmpl/libstdc++.scr
  INCLUDE config.tmpl/libsupc++.scr

/* NEWLIB */
  INCLUDE config.tmpl/libc.scr
  INCLUDE config.tmpl/libm.scr

/* RTEMS */

  /* libmisc -- UNUSED */
    /* INCLUDE config.tmpl/libmisc.scr */

  /* libnetapps -- DONT USE; this doesn't belong into RTEMS core */
    /* INCLUDE config.tmpl/libnetapps.scr */

  /* libblock & friends -- I dont like it nor need it */
    /* INCLUDE config.tmpl/libblock.scr */
    /* INCLUDE config.tmpl/libdosfs.scr */

  /* IMFS ('in memory file system') -- NEEDED */
    INCLUDE config.tmpl/libimfs.scr

  /* libchip -- don't enforce linkage */
    /* INCLUDE config.tmpl/libnetchip.scr */
    /* INCLUDE config.tmpl/librtcio.scr */
    /* INCLUDE config.tmpl/libserialio.scr */

  /* networking -- NEEDED */
    INCLUDE config.tmpl/libnetworking.scr
  /* RPC */
    INCLUDE config.tmpl/librpc.scr
    INCLUDE config.tmpl/libxdr.scr
  /* RDBG (remote debugging support) -- is broken but I leave it for the moment */
    INCLUDE config.tmpl/librdbg.scr

  /* RTEMS CORE */
    INCLUDE config.tmpl/librtems.scr
    INCLUDE config.tmpl/libsapi.scr
    INCLUDE config.tmpl/libposix.scr
    INCLUDE config.tmpl/libscore.scr
    INCLUDE config.tmpl/libcsupport.scr
    INCLUDE config.tmpl/librtems++.scr

  /* BSP CORE */
    INCLUDE config.tmpl/libbsp.scr

/* SITE SPECIFIC LIBRARIES */
  INCLUDE config.tmpl/libbspExt.scr
  INCLUDE config.tmpl/libtecla_r.scr
  INCLUDE config.tmpl/libspencer_regexp.scr
