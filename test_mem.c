/* Try to force the system to free memory. Write a program, that will allocate 
 * a lot of virtual memory and touch each page. That will force the kernel to 
 * free anything that can be freed.
 */

#include <stdlib.h>
#include <assert.h>
#include <time.h>

#define KB *1024
#define MB *1024 KB

int main()
{
  struct timespec ts={.tv_sec=1,.tv_nsec=300000000};
  int i;
  char *buf;
  assert(buf=malloc(400 MB));
  for (i=0;i<400 MB;i+=4 KB)
    buf[i]++;
  nanosleep(&ts,NULL);
  return 0;
}

