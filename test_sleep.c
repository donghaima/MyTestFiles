#define _POSIX_C_SOURCE 199309 

#include <time.h>
#include <stdio.h>

int msleep (unsigned long milisec)   
{   
    struct timespec req={0};   
    time_t sec=(int)(milisec/1000);   
    milisec=milisec-(sec*1000);   
    req.tv_sec=sec;   
    req.tv_nsec=milisec*1000000L;   
    while(nanosleep(&req,&req)==-1)   
         continue;   
    return 1;   
}   

int main (void)
{
    int i;
    for (i=0; i<3000; i=i+100) {
        printf("sleep for %d milliseconds...\n", i);
        msleep(i);
    }
    return 0;
}

