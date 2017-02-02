
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define _GNU_SOURCE
#include <getopt.h>


/* Use long form for all options except 'h' and 'v'*/
static struct option long_opt[] = {
    {"xmlrpc-port", 1, NULL, 0},
    {"dscp", 1, NULL, 0},
    {"do-mi-check", 0, NULL, 0},
    {"rcv-buflen", 1, NULL, 0},
    {"snd-buflen", 1, NULL, 0},
    {"log-level", 1, NULL, 0},
    {"version", 0, NULL, 'v'},
    {"help", 0, NULL, 'h'},
    {0, 0, 0, 0}
};


typedef uint8_t boolean;

typedef struct stunsvr_opt_ {
    uint16_t xmlrpc_port;
    char xmlrpc_log[256];
    int log_level;
    boolean do_mi_check;
    uint8_t dscp;
    uint32_t rcv_bufflen;
    uint32_t snd_bufflen;
} stunsvr_opt;

static stunsvr_opt options;

int main (int argc, char *argv[])
{
    /* Process the command line arguments */
    int c = 0, tmp;
    int option_index = 0;

    const char *optstr = "vh";

    while ((c = getopt_long(argc, argv, optstr, long_opt, &option_index)) 
           != EOF) {

        switch( c ) {  

        case 0:     /* long option without a short arg */
            if (strcmp( "log-level", long_opt[option_index].name) == 0) {
                tmp = atoi(optarg);
                options.log_level = tmp;

                printf("log-level: option_index=%d, name=%s, optarg=%s\n",
                       option_index, long_opt[option_index].name, optarg);

            } else if (strcmp("dscp", long_opt[option_index].name) == 0) {
                options.dscp = (uint8_t)strtol(optarg, NULL, 16);

                printf("dscp: option_index=%d, name=%s, optarg=%s, dscp=0x%x\n",
                       option_index, long_opt[option_index].name, optarg,
                       options.dscp);


            } else {

                printf("else: option_index=%d, name=%s, optarg=%s\n",
                       option_index, long_opt[option_index].name, optarg);

            }
            break;

#if 0
            options.xmlrpc_port = atoi(optarg);

            options.do_mi_check = 1;

            options.dscp = (uint8_t)strtol(optarg, NULL, 16); 

            options.rcv_bufflen = tmp;

            options.snd_bufflen = tmp;
#endif

            
        case 'v':  /* version number */
            printf("\nSTUN Server: version 2.1.0\n");
            return 0;
            
        case 'h':  /* help */
            printf( "\nUsage: %s [options]\n", argv[0]);
            return -1;

        default:   /* invalid options or missing arguments */
            fprintf(stderr, "Invalid options %s\n", argv[optind-1]);
            break;
        } /* switch (c) */
    } /* while (c=getopt_long() ...) */

    if ((argc - optind) > 0) {
        /* Should log a warning message and continue */
        fprintf(stderr, "Missing arguments\n");
    }

    printf("test_getopt finished\n");

    return 0;
}
