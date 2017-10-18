#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>

#include <limits.h>

extern const char *rtems_bsdnet_domain_name;

#ifndef DEBUG_MAIN
#include <reent.h>
extern void __env_lock(struct _reent *);
extern void __env_unlock(struct _reent *);
#ifdef HAVE_LIBBSD /* don't know how to get that info easily */
#define rtems_bsdnet_domain_name ""
#endif
#else
#define __env_lock(x)   do { } while (0)
#define __env_unlock(x) do { } while (0)
static char dombuf[100];
const char *rtems_bsdnet_domain_name = dombuf;
#endif

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

static char * cwdbuf(unsigned pre, unsigned post)
{
int   sz, xtra;
char *rval = 0;

	xtra = pre + post;

	for ( sz = 16; sz < xtra + 16; sz *= 2 )
		;

	do {
		if ( ! (rval = realloc(rval, sz)) ) {
			return 0;
		}

		if ( getcwd(rval + pre, sz - xtra) )
			return rval;

		sz *= 2;

	} while ( ERANGE == errno && sz <= _POSIX_PATH_MAX );

	free(rval);

	return 0;
}

/* Add CWD with suffix to PATH */
int
addpathcwd(char *suffix, int prepend)
{
int     l = suffix ? (strlen(suffix) + 1) : 0;
char *buf = 0; 
int  rval = -1;
	
	if ( ! (buf = cwdbuf(0, l)) )
		goto cleanup;

	if ( suffix )
		strcat(buf, suffix);

	rval = addpath(buf, prepend);

cleanup:
	free(buf);
	return rval;
}

/* Allocate a new string substituting all '%<tagchar>' occurrences
 * in 'p' by substitutions listed in s[].
 * If no matching substitution is found, %<tagchar> is copied to
 * the destination string verbatim.
 * '%%' is expanded to a single '%' character.
 * 's' is a list of substitution strings with the first
 * character being the 'tag char' followed by the substitution
 * string/expansion.
 * It is the user's responsibility to free the returned string.
 *
 *  'p': template (input) with '%<tagchar>' occurrences
 *  's': array of 'ns' "<tagchar><subst>" strings
 * 'ns': number of substitutions
 *
 * RETURNS: malloc()ed string with expanded substitutions on
 *          success, NULL on error (NULL p, no memory).
 *      
 *
 * Example: 
 *    char *s[] = { "wworld" };
 *
 *    newp = stringSubstitute("hello %w %x %%",s,1)
 *
 *    generates the string newp -> "hello world %x %"
 */

char *
stringSubstitute(const char *p, const char * const *s, int ns)
{
	register const char *pp;
	register char       *dd;
	char *rval;
	int l,i,ch;

	if ( !p )
		return 0;

	for ( l = 0, rval = dd = 0; !rval; ) {
		if ( l ) {
			if ( ! (dd = rval = malloc(l+1)) )
				return 0;
		}
		for ( pp=p; *pp; pp++ ) {
			if ( '%' != *pp ) {
				if ( dd )
					*dd++ = *pp;
				else
					l++;
			} else {
				const char *ptmp = pp;

				if ( (ch = *(pp+1)) ) {

					if ( '%' == ch ) {
						pp++;
					} else {

						for ( i = 0; i<ns; i++ ) {
							if ( *s[i] == ch ) {
								/* don't count initial substitution char
								 * subtract 1 more char; will be added
								 * again a few lines below...
								 */
								int ll = strlen(s[i]) - 1 - 1;
								if ( dd ) {
									strcpy(dd, s[i]+1);	
									dd  += ll;
									ptmp = dd;
								} else {
									l   += ll;
								}
								pp++;
								break;
							}
						}
						/* no substitution found; treat as ordinary '%'
						 * (in case a substitution was found, the extra
						 * char added here had been subtracted above)
						 */
					}
				}
				/* if % was the last char then this will just copy it */
				if ( dd ) {
					*dd++ = *ptmp;
				} else {
					l++;
				}
			}
		}
	}
	*dd++ = 0;
	assert( dd == rval + l + 1);
	return rval;
}

/* A vararg wrapper */

char *
stringSubstituteVa(const char *p, ...)
{
va_list    ap;
int        ns;
const char **sp = 0;
const char *cp;
char       *rval;

	/* count number of substitutions */
	va_start(ap,p);
	for (ns = 0; va_arg(ap, const char *); ns++)
		;
	va_end(ap);
	if ( ns && ! (sp = malloc(sizeof(*sp) * ns)) )
		return 0;
	va_start(ap,p);
	for (ns = 0; (cp = va_arg(ap, const char *)); ns++)
		sp[ns] = cp;
	va_end(ap);

	rval = stringSubstitute(p, sp, ns);

	free(sp);

	return rval;
}


/* Allocate a string and copy the template 'tmpl'
 * making the following substitutions:
 *
 *        %H -> hostname ('gethostname')
 *        %D -> domainname ('getdomainname')
 *        %P -> cwd ('getcwd')
 *        %I -> IP address (dot notation) [NOT SUPPORTED YET -- it's not trivial to find our IP address].
 *
 * RETURNS: newly allocated string (user must free())
 *          or NULL (no memory or gethostname etc. failure).
 */

#define MAX_SUBST 4

#define MAX_NAM 100

char *
pathSubstitute(const char *tmpl)
{
int  i;
char *s[MAX_SUBST];
int     ns = 0;
char *rval = 0;
char * const *sp = s;

	for ( i=0; i<MAX_SUBST; i++ )
		s[i] = 0;

	if ( strstr(tmpl,"%H") ) {
		if ( ! (s[ns] = malloc(MAX_NAM)) )
			goto bail;
		s[ns][0]='H';	/* tag char */
		if ( gethostname(s[ns]+1,MAX_NAM - 1) ) {
			/* gethostname failure */
			goto bail;
		}
		ns++;
	}
	if ( strstr(tmpl,"%D") ) {
		if ( ! (s[ns] = malloc(MAX_NAM)) )
			goto bail;
		s[ns][0]='D';	/* tag char */
		if ( rtems_bsdnet_domain_name ) {
			strncpy(s[ns]+1,rtems_bsdnet_domain_name, MAX_NAM - 1);
			s[ns][MAX_NAM - 1] = 0;
		} else {
			s[ns][1]=0;
		}
		ns++;
	}
	if ( strstr(tmpl,"%P") ) {
		if ( ! (s[ns] = cwdbuf(1,0)) )
			goto bail;
		s[ns][0] = 'P';
		ns++;
	}

	rval = stringSubstitute(tmpl, (const char * const *)sp, ns);

bail:
	for ( i = 0; i<MAX_SUBST; i++ ) {
		free(s[i]);
	}
	return rval;
}

/* chdir to a path containing %H / %D substitutions */
int
chdirTo(const char *tmpl)
{
char *p = pathSubstitute(tmpl);
int  rval = -1;
	if ( p )
		rval = chdir(p);
	free(p);
	return rval;
}


#ifdef DEBUG_MAIN

#include <stdio.h>

static void
usage(char *nm)
{
	fprintf(stderr,"Usage: %s [a] [-s <subst>] [-s <subst>] <path>\n",nm);
	fprintf(stderr,"       <subst> : 'subst_char' 'subst_string', e.g., Hhhh\n");
}

int main(int argc, char **argv)
{
const char *s[20];
int   ns = 0;
char *p;
int   ch;

	getdomainname(dombuf, sizeof(dombuf));

	while ( (ch = getopt(argc, argv, "as:")) > 0 ) {
		switch ( ch ) {
			case 'a':
				addpathcwd(":",1);
				printf("PATH=%s\n",getenv("PATH"));
			break;
			case 's':
				if ( ns < sizeof(s)/sizeof(s[0]) ) {
					s[ns]=optarg;
					ns++;
				} else {
					fprintf(stderr,"Too many substitutions; skipping\n");
				}
			break;
			default:
				fprintf(stderr,"Unknown option '%c'\n",ch);
				usage(argv[0]);
			break;
		}
	}

	if ( optind >= argc ) {
		fprintf(stderr,"Path argument missing\n");
		usage(argv[0]);
		return(1);
	}

	if ( ns > 0 ) {
		if ( !(p = stringSubstitute(argv[optind],s,ns)) ) {
			fprintf(stderr,"stringSubstitute failed\n");
			return(1);
		}
	} else {
		if ( !(p = pathSubstitute(argv[optind])) ) {
			fprintf(stderr,"pathSubstitute failed\n");
			return(1);
		}
	}
	printf("%s\n",p);
	free(p);
}
#endif

