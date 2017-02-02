/* To build: gcc -g -o test_rpc test_rpc.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <errno.h>


#define ASSERT(exp,...) \
{ if (!(exp)) {fprintf(stderr, "FAILED ASSERTION\n");\
      fprintf(stderr,__VA_ARGS__); fprintf(stderr,"\n"); abort(); } }

#define VAMIPC_KEY_FILE_DEF ".vamipc_key_file"

#define TRUE 1
#define FALSE 0

key_t vam_ipc_get_key(const char * ipcf, int proj_id)
{
    char * malloced = NULL;
    key_t result = -1;

    if (! ipcf) {
        struct passwd * pwent = getpwuid(getuid());
        if (!pwent) {
            fprintf(stderr, "Could not get pw entry\n");
            goto bail;
        }

        const char * homedir = pwent->pw_dir; 
        malloced = malloc(strlen(homedir) + strlen(VAMIPC_KEY_FILE_DEF) + 2);
        if (! malloced)
            goto bail;
        sprintf(malloced, "%s/%s",homedir,VAMIPC_KEY_FILE_DEF);
        ipcf = malloced;
    }

    FILE * f = NULL;

    /* Make sure the ftok file exists or create it */
    if ((f = fopen(ipcf,"r"))
        || (f = fopen(ipcf, "w+"))) {
        ASSERT(fclose(f) == 0, "Could not close file");
    } else {
        fprintf(stderr, "Could not open %s\n",ipcf);
        goto bail;
    } 

    result = ftok(ipcf, proj_id);

 bail: 
    if (malloced)
        free(malloced);
    return result;
}



int main(void)
{
    int status = TRUE;
    int req_msgid = 0;
    key_t req_key = vam_ipc_get_key(NULL, 1);

    if (req_key < 0) {
        fprintf(stderr, "Failed to get IPC key\n");
        return -1;
    }

    do { 

        /********************************************************************
         * For the server we want to start in a known state(i.e. force
         * a create).  If the queue exists delete it, then create a
         * new one.
         ********************************************************************/
        /********************* 
         * the Request queue 
         *********************/
        printf("Before...\n");
        system("ipcs -q|grep doma");

        if ((req_msgid = msgget(req_key,0)) == -1) {
            if (errno != ENOENT) {
                status = FALSE /*$$RPC$ */;
                break;
            } 

            fprintf(stderr, "msgget() < 0: errno=%d, %m - status=%d\n", 
                    errno, status);

        }
        else {
            if (msgctl(req_msgid, IPC_RMID, 0) == -1){
                status = FALSE/*$$RPC$ */;

                fprintf(stderr, "msgctl() < 0: %m\n");
                break;
            }

            printf("msgctl() removed existing msgid %d: %m\n", req_msgid);
            system("ipcs -q|grep doma");

        }
        if ((req_msgid = msgget(req_key,(IPC_CREAT|0660))) == -1) {
            status = FALSE;
            fprintf(stderr, "msgget() could not create request queue: %m\n");
            break; 
        }

        printf("msgget() created request queue: %m\n");
        system("ipcs -q|grep doma");

    } while(0);

    if (! status) {
        fprintf(stderr, "Something is wrong!\n");
        return -1;
    }

    return 0;
}
