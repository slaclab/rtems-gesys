/* $Id$ */

#include <reent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern void __env_lock(struct _reent *);
extern void __env_unlock(struct _reent *);

/* append/prepend to environment var */
int
addenv(char *var, char *val, int prepend)
{
char *buf;
char *a,*b;
int  len, rval;

	if ( !var || !val ) {
		return -1;
	}

	__env_lock(_REENT);

	if ( ! (a=getenv(var)) ) {
		rval = setenv(var, val, 0);
		__env_unlock(_REENT);
		return rval;
	}
	len  = strlen(val) + strlen(a) + 1 /* terminating 0 */;

	if ( !(buf = malloc(len)) ) {
		__env_unlock(_REENT);
		return -1;
	}

	if ( prepend ) {
		b = a;
		a = val;
	} else {
		b = val;
	}
	strcpy(buf,a);
	strcat(buf,b);
	rval = setenv(var, buf, 1);
	__env_unlock(_REENT);

	free(buf);

	return rval;	
}

/* Add to PATH */
int
addpath(char *val, int prepend)
{
	return addenv("PATH",val,prepend);
}

/* Add CWD with suffix to PATH */
int
addpathcwd(char *suffix, int prepend)
{
int     l = suffix ? (strlen(suffix) + 1) : 0;
char *buf = 0; 
int  rval = -1;
	
	l += MAXPATHLEN;
	if ( ! (buf = malloc(l)) )
		goto cleanup;

	if ( ! getcwd(buf, MAXPATHLEN) )
		goto cleanup;

	if ( suffix )
		strcat(buf, suffix);

	rval = addpath(buf, prepend);

cleanup:
	free(buf);
	return rval;
}
