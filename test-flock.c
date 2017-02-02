#include <stdio.h>
#include <fcntl.h>

int getLock(void)
{
  struct flock fl;
  int fdlock;

  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = 0;
  fl.l_len = 0;

  if((fdlock = open("/var/run/ChannelManager.pid", O_RDWR|O_CREAT, 0600)) < 0)
    return 0;

  if(fcntl(fdlock, F_SETLK, &fl) <0)
    return 0;

  return 1;
                        
}


int main (void)
{
	if (!getLock())
	{
		printf("Already running\n");
		return -1;
	}

	printf("First time running\n");
	return 0;

}


