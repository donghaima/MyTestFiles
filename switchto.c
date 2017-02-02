/* http://kasperd.net/~kasperd/linux_kernel/switchto.c  */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/vt.h>

int main(int argc, char ** argv) {
  int fd,t,vt=0;
  if (argc == 2) vt=atoi(argv[1]);
  if ((vt<1)||(vt>63)) {
    fprintf(stderr,"Usage: switchto vt_num\n");
  } else {
    for (t=0;t<64;++t) {
      char buf[42];
      sprintf(buf,"/dev/tty%d",t);
      fd=open(buf,O_WRONLY);
      if (fd!=-1) {
	sprintf(buf,"\x1B[12;%d]",vt);
	ioctl(fd,VT_DISALLOCATE,0);
	write(fd,buf,strlen(buf));
	ioctl(fd,VT_DISALLOCATE,0);
	return 0;
      }
    }
    fprintf(stderr,"switchto: Can't open any tty\n");
  }
  return 1;
}

