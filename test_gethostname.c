#include <stdio.h>
#include <unistd.h>
#include <string.h>

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

static char vqes_hname[MAXHOSTNAMELEN + 1];

/* Get the vqes hostname */
int setup_vqes_hostname(void)
{
    int ret = 0;
    ret = gethostname(vqes_hname, MAXHOSTNAMELEN);
    if (ret < 0) {
        /* Set to a default name if failed to get host name */
        strncpy(vqes_hname, "vqe-server", MAXHOSTNAMELEN);
    }
    return ret;
}

/* Copy over to the specified string buffer */
char* vqes_hostname(char *hn, size_t n)
{
    if (hn) {
        return strncpy(hn, vqes_hname, n);
    }
    return NULL;
}


int main(void)
{
    char hname[128] = "vqe-server";
    if (gethostname(hname, sizeof(hname)) < 0) {
        //if (gethostname(hname, 1) < 0) {   /* error case */
        printf("Failed to get vqes_hostname: %m\n");
    }
    printf("host name=%s\n", hname);

    int ret = setup_vqes_hostname();
    if (ret < 0) {
        printf("Failed to get vqes_hostname: %m\n");
        return (-1);
    }

    printf("VQES host name=%s\n", vqes_hostname(hname, sizeof(hname)));
    return 0;
}
