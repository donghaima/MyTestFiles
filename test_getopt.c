
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define _GNU_SOURCE
#include <getopt.h>


static struct option long_opt[] = {
    {"xmlrpc-port", 1, NULL, 'x'},
    {"dscp", 1, NULL, 's'},
    {"do-mi-check", 0, NULL, 'm'},
    {"rcv-buflen", 1, NULL, 'R'},
    {"snd-buflen", 1, NULL, 'S'},
    {"log-level", 1, NULL, 'L'},
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


    const char *optstr = "x:s:L:vhmR:S:";

    while ((c = getopt_long_only(argc, argv, optstr, long_opt, &option_index)) 
           != EOF) {

        switch( c ) {  

        case 'x':  /* port for xmlrpc server */
            printf("optind=%d, option name=%s, optarg=%s\n",
                   optind, argv[optind-2], optarg);

            options.xmlrpc_port = atoi(optarg);
            break;

        case 'm':  /* enable STUN message integrity checking */
            options.do_mi_check = 1;
            break;

        case 's':  /* DSCP: expect a hex value */
            options.dscp = (uint8_t)strtol(optarg, NULL, 16); 
            break;

        case 'R':  /* recv buffer size */
            tmp = atoi(optarg);
            options.rcv_bufflen = tmp;
            break;

        case 'S':  /* send buffer size */
            tmp = atoi(optarg);
            options.snd_bufflen = tmp;
            break;
            
        case 'v':  /* version number */
            printf("\nSTUN Server: version 2.1.0\n");
            return 0;

        case 'L':  /* log level */
            printf("optind=%d, option name=%s, optarg=%s\n",
                   optind, argv[optind-2], optarg);

            printf("option_index=%d, name=%s, optarg=%s\n",
                   option_index, long_opt[option_index].name, optarg);

            printf("option_index=%d, name=%s, optarg=%s, optind=%d\n",
                   option_index, opt_name, optarg, optind
            
            tmp = atoi(optarg);
            options.log_level = tmp;
            break;
            
        case 'h':  /* help */
            printf( "\nUsage: %s [options]\n", argv[0]);
            return -1;

        default:   /* invalid options or missing arguments */
            fprintf(stderr, "Invalid options %s\n", argv[optind-2]);
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
