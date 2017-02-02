#define _POSIX_C_SOURCE 200112L

#include <limits.h>
#include <unistd.h>
#include <stdio.h>

int main( int argc, char ** argv )
{
   long x = sysconf( _SC_THREAD_THREADS_MAX );

   printf( "%li\n", x );

   return 0;
}
