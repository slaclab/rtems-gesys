#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int
cexpModuleLoad(char*,char*);

void
cexpFlashSymLoad(unsigned char bank)
{
unsigned char	*flashctrl=(unsigned char*)0xffeffe50;
unsigned char	*flash    =(unsigned char*)0xfff80000;
int				fd;
struct	stat	stbuf;
int				len,i,written;
char 			tmpfname[30]={
				        '/','t','m','p','/','m','o','d','X','X','X','X','X','X',
					        0};

	/* switch to desired bank */
	*flashctrl=(1<<7)|bank;

	if (stat("/tmp",&stbuf)) {
		mode_t old = umask(0);
		mkdir("/tmp",0777);
		umask(old);
	}
	if ( (fd=mkstemp(tmpfname)) < 0) {
		perror("creating scratch file");
		goto cleanup;
	}

	len=*(unsigned long*)flash;
	flash+=sizeof(unsigned long);

	printf("Copying %i bytes\n",len);
	for (i=written=0; i < len; i+=written ) {
		written=write(fd,flash+i,len-i);
		if (written<=0)
			break;
	}
	close(fd);
	if (written<0) {
		perror("writing to tempfile\n");
	} else {
		printf("OK, ready to load\n");
		cexpModuleLoad(tmpfname,0);
	}

cleanup:
	unlink(tmpfname);
}
