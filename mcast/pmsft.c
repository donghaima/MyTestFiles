/***
 *
 * pmsft - test a IGMPv3 host stack, using the protocol independent api
 *
 * Usage:       pmsft [filename] ; '?' shows help
 *
 * File:        pmsft.c
 * Date:        12/4/2000
 * Auth:        wilbertdg@hetnet.nl
 *
 *
 * Remarks:
 * In order to compile, your OS needs to support the 'Socket Interface Extensions 
 * for Multicast Source Filters' as specified in 'draft-ietf-idmr-msf-api-01.txt'.
 *
 ***/

#define MAX_ADDRS		500

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

/*
 * Mapping from error to symblic name
 */
static struct err_entry {
	int err_no;
	char *err_name;
} err_map [] = {
	{ EINVAL,		 	"EINVAL" },
	{ ENOENT, 			"ENOENT" },
	{ ENOMEM, 			"ENOMEM" },
	{ EADDRNOTAVAIL, 	"EADDRNOTAVAIL" },
	{ 0,				"ENOERR" }
};

/*
 * Prototypes
 */
void process_file(char *, int);
void process_cmd(char*, int, FILE *fp);
char *error_2_name(int err);
void usage();
int comp_sas(const void *, const void *);

main( argc, argv )
    int argc;
    char **argv;
  {
    int s, i;
    char line[80], *p;

	/* Create the socket that's used for all commands
	 */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		exit(1);
	}

	if (argc < 2) {
		printf("> ");
		/* Process commands until user wants to quit
		 */
   		 while (1) {
 			if (fgets(line, sizeof(line), stdin) != NULL) {
				if (line[0] != 'f')
					process_cmd(line, s, stdin);
				else {
					/* Get the filename */
					for (i=1; isblank(line[i]); i++);
					if ((p = (char*) strchr(line, '\n')) != NULL)
						*p = '\0';
					process_file(&line[i], s);
				}
			}
		}
	}
	else {
		/* Process the files that were specified for arguments
		 */
		for (i=1; i<argc; i++)
			process_file(argv[i], s);
	}
}

/*
 * Process commands from a file
 */
void process_file(fname, s)
	char *fname;
	int s;
{
    char *lineptr, *p;
    char line[80];
	FILE *fp;

	/* Try to open the file and feed all commands to process_cmd() */
	fp = fopen(fname, "r");
	if (fp != NULL) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			/* Skip comments and empty lines */
			lineptr = line;
			while (isblank(*lineptr)) lineptr++;
			if (*lineptr != '#' && *lineptr != '\n')
				process_cmd(lineptr, s, fp);
		}
		fclose(fp);
	}
	else
		printf("%s\n", error_2_name(ENOENT));
}

/*
 * Process a line/command
 */
void process_cmd(cmd, s, fp)
	char *cmd;
	int s;
	FILE *fp;
{
	struct ifreq ifr;
	struct group_req gr;
	struct group_source_req gsr;
	struct group_filter *gfp;
	struct sockaddr_in sin, sin1, sin2, sin3, *psin;
	char buffer[GROUP_FILTER_SIZE(MAX_ADDRS)];
	char str1[20], str2[20], str3[20], *line, *p;
	int i, n, opt, f;

	/* Skip whitespaces */
	line = cmd + 1;
	while (isblank(*line)) line++;

	switch (*cmd) {

		case '?':
			/* Show usage
			 */
			usage();
			break;

		case 'q':
			/* Quit
			 */
			close(s);
			exit(0);

		case 's':
			/* Wait for some time
			 */
			if ((sscanf(line, "%d", &n) != 1) || (n < 1)) {
				printf("-1\n");
				break;
			}
			for (i=0; i<n; i++)
				sleep(1);
			break;

		case 'j':
		case 'l':
			/* Join or leave a multicast group
			 */
			if (((sscanf(line, "%s %d", str1, &n)) != 2) ||
				((sin1.sin_addr.s_addr = inet_addr(str1)) == INADDR_NONE)) {
				printf("-1\n");
				break;
			}
			psin = (struct sockaddr_in*) &gr.gr_group;
			psin->sin_family = AF_INET;
			psin->sin_addr.s_addr = sin1.sin_addr.s_addr;
			gr.gr_interface = n;
			opt = (*cmd == 'j') ? MCAST_JOIN_GROUP : MCAST_LEAVE_GROUP;
			if (setsockopt(s, IPPROTO_IP, opt, &gr, sizeof(gr)) == -1)
				printf("%s\n", error_2_name(errno));
			else
				printf("%s\n", error_2_name(0));
			break;

		case 'a':
		case 'd':
			/* Add or delete a multicast ethernet address to some interface
			 */
			sscanf(line, "%s %s", str1, str2);
			p = str2;
			for (i=0; i<6 && p!=NULL; i++) {
				if ((p = strtok(p, ".")) != NULL) {
					ifr.ifr_addr.sa_data[i] = atoi(p);
					p++;
				}
			}
			opt = (*cmd == 'a') ? SIOCADDMULTI : SIOCDELMULTI;
			if (ioctl(s, opt, &ifr) == -1)
				printf("%s\n", error_2_name(errno));
			else
				printf("%s\n", error_2_name(0));
			break;

		case 'm':
		case 'p':
			/* Set or clear the multicast or promisc flag
			 */
			if (sscanf(line, "%s %u", ifr.ifr_name, &f) != 2) {
				printf("-1\n");
				break;
			}
			if (ioctl(s, SIOCGIFFLAGS, &ifr) == -1) {
				printf("%s\n", error_2_name(errno));
				break;
			}
			opt = (*cmd = 'm') ? IFF_ALLMULTI : IFF_PROMISC;
			if (f)
				ifr.ifr_flags |= opt;
			else
				ifr.ifr_flags &= ~opt;
			if (ioctl(s, SIOCSIFFLAGS, &ifr) == -1)
				printf("%s\n", error_2_name(errno));
			else
				printf("%s\n", error_2_name(0));
			break;

		case 'i':
		case 'e':
			/* Set the socket to include or exclude filter mode, and
			 * add some sources to the filterlist, using the full-state,
			 * or advanced api 
			 */
			if (((sscanf(line, "%s %d %d", str1, &i, &n)) != 3) ||
				((sin1.sin_addr.s_addr = inet_addr(str1)) == INADDR_NONE) ||
				(n > MAX_ADDRS)) {
				printf("-1\n");
				break;
			}
			/* Prepare argument 
			 */
			gfp = (struct group_filter*) buffer;
			psin = (struct sockaddr_in*) &gfp->gf_group;
			psin->sin_family = AF_INET;
			psin->sin_addr.s_addr = sin1.sin_addr.s_addr;	
			gfp->gf_fmode = (*cmd == 'i') ? MCAST_INCLUDE : MCAST_EXCLUDE;
			gfp->gf_interface = i; 
			gfp->gf_numsrc = n;
			for (i=0; i<n; i++) {
				fgets(str1, sizeof(str1), fp);
				psin = (struct sockaddr_in*) &gfp->gf_slist[i];
				psin->sin_family = AF_INET;
				if ((psin->sin_addr.s_addr = inet_addr(str1)) == INADDR_NONE) {
					printf("-1\n");
					return;
				}
			}
			/* Execute ioctl()
			 */
			if (ioctl(s, SIOCSMSFILTER, (void*) gfp) != 0)
				printf("%s\n", error_2_name(errno));
			else
				printf("%s\n", error_2_name(0));
			break;

		case 't':
		case 'b':
			/* Allow or block traffic from a source, using the delta based api
			 */
			if (((sscanf(line, "%s %d %s", str1, &i, str2)) != 3) ||
				((sin1.sin_addr.s_addr = inet_addr(str1)) == INADDR_NONE) ||
				((sin2.sin_addr.s_addr = inet_addr(str2)) == INADDR_NONE)) { 
				printf("-1\n");
				break;
			}
			/* First find out current filter mode 
			 */
			gfp = (struct group_filter*) buffer;
			psin = (struct sockaddr_in*) &gfp->gf_group;
			psin->sin_family = AF_INET;
			psin->sin_addr.s_addr = sin1.sin_addr.s_addr;	
			gfp->gf_interface = i; 
			gfp->gf_numsrc = 0;
			if (ioctl(s, SIOCGMSFILTER, gfp) != 0) {
				/* It's only okay for 't' to fail, since the operation
				 * MCAST_JOIN_SOURCE_GROUP on a non existing membership
				 * should result in a new membership
				 */
				if (*cmd != 't') {
					printf("%s\n", error_2_name(errno));
					break;
				}
				else
					gfp->gf_fmode = MCAST_INCLUDE;
			}
			if (gfp->gf_fmode == MCAST_EXCLUDE) {
				/* Any source */
				opt = (*cmd == 't') ? 
					MCAST_UNBLOCK_SOURCE : MCAST_BLOCK_SOURCE;
			}
			else {
				/* Controlled source */
				opt = (*cmd == 't') ? 
					MCAST_JOIN_SOURCE_GROUP : MCAST_LEAVE_SOURCE_GROUP;
			}
			/* Prepare argument
			 */
			gsr.gsr_interface = i;
			psin = (struct sockaddr_in*) &gsr.gsr_group;
			psin->sin_family = AF_INET;
			psin->sin_addr.s_addr = sin1.sin_addr.s_addr;
			psin = (struct sockaddr_in*) &gsr.gsr_source;
			psin->sin_family = AF_INET;
			psin->sin_addr.s_addr = sin2.sin_addr.s_addr;
			/* Execute setsockopt()
			 */ 
			if (setsockopt(s, IPPROTO_IP, opt, &gsr, sizeof(gsr)) == -1)
				printf("%s\n", error_2_name(errno));
			else
				printf("%s\n", error_2_name(0));
			break;

		case 'g':
			/* Get and show the current filter mode, and the sources in the list
			 */
			if (((sscanf(line, "%s %d %d", str1, &i, &n)) != 3) ||
				((sin1.sin_addr.s_addr = inet_addr(str1)) == INADDR_NONE)) { 
				printf("-1\n");
				break;
			}
			/* Prepare argument
			 */
			gfp = (struct group_filter*) buffer;
			psin = (struct sockaddr_in*) &gfp->gf_group;
			psin->sin_family = AF_INET;
			psin->sin_addr.s_addr = sin1.sin_addr.s_addr;	
			gfp->gf_interface = i; 
			gfp->gf_numsrc = n;
			/* Execute ioctl() 
			 */
			if (ioctl(s, SIOCGMSFILTER, gfp) != 0) {
				printf("%s\n", error_2_name(errno));
				break;
			}
			printf("%s\n", (gfp->gf_fmode == MCAST_INCLUDE) ? 
					"include" : "exclude");
			if (n > gfp->gf_numsrc) {
				n = gfp->gf_numsrc;
				qsort(gfp->gf_slist, n, sizeof(struct sockaddr_storage), &comp_sas);
				for (i=0; i<n; i++) {
					psin = (struct sockaddr_in*) &gfp->gf_slist[i];
					printf("%s\n", inet_ntoa(psin->sin_addr));
				}
			}
			break;

		case '\n':
			break;

		default:
			break;
	}
}

/*
 * Print usage information
 */
void usage()
{
	printf("j g.g.g.g i                - join IP multicast group\n");
	printf("l g.g.g.g i                - leave IP multicast group\n");
	printf("a ifname e.e.e.e.e.e       - add ether multicast address\n");
	printf("d ifname e.e.e.e.e.e       - delete ether multicast address\n");
	printf("m ifname 1/0               - set/clear ether allmulti flag\n");
	printf("p ifname 1/0               - set/clear ether promisc flag\n");
	printf("i g.g.g.g i n              - set n include mode src filters\n");
	printf("e g.g.g.g i n              - set n exclude mode src filters\n");
	printf("t g.g.g.g i s.s.s.s        - allow traffic from src\n");
	printf("b g.g.g.g i s.s.s.s        - block traffic from src\n");
	printf("g g.g.g.g i n              - get and show (max n) src filters\n");
	printf("f filename                 - read command(s) from file\n");
	printf("s seconds                  - sleep for some time\n");
	printf("q                          - quit\n");
}
/*
 * Try to translate an error message in the name of the define 
 */
char *error_2_name(int err)
{
	int i;
	for (i=0; err_map[i].err_no != 0; i++)
		if (err_map[i].err_no == err)
			return err_map[i].err_name;
	return (err != 0) ? "UNKNOWN" : "ENOERR";
}

int comp_sas(const void *a, const void *b)
{
	struct sockaddr_in *sa, *sb;
	sa = (struct sockaddr_in*) a;
	sb = (struct sockaddr_in*) b;
	return (int) sa->sin_addr.s_addr - sb->sin_addr.s_addr;
}

