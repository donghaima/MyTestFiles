#include <stdio.h>
    #include <stdlib.h>
    #include <ctype.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>

    int main(void)
    {
        int sv[2]; /* the pair of socket descriptors */
        char buf; /* for data exchange between processes */

        socketpair(AF_UNIX, SOCK_STREAM, 0, sv);

        if (!fork()) {  /* child */
            read(sv[1], &buf, 1);
            printf("child: read '%c'\n", buf);
            buf = toupper(buf);  /* make it uppercase */
            write(sv[1], &buf, 1);
            printf("child: sent '%c'\n", buf);

        } else { /* parent */
            write(sv[0], "b", 1);
            printf("parent: sent 'b'\n");
            read(sv[0], &buf, 1);
            printf("parent: read '%c'\n", buf);
        }
        return 0;
    }

