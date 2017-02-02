/* 
 *  gcc -g -o test_random test_random.c -lssl
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <openssl/md5.h>

#define RANDOM_GENERIC_TYPE   1
#define RANDOM_SSRC_TYPE      1
#define RANDOM_TIMESTAMP_TYPE 2
#define RANDOM_SEQUENCE_TYPE  3


/* Random Generation. Taken from RFC 3550*/
//#define MD_CTX struct MD5_CTX
//#define MDInit MD5_Init
//#define MDUpdate MD5_Update
//#define MDFinal MD5_Final

static uint32_t md_32(char *string, int length)
{
    MD5_CTX context;
    union {
        unsigned char c[16];
        uint32_t x[4];
    } digest;
    uint32_t r;
    int32_t i;
    MD5_Init (&context);
    MD5_Update (&context, (unsigned char *)string, length);
    MD5_Final ((unsigned char *)&digest, &context);
    r = 0;
    for (i = 0; i < 3; i++) {
        r ^= digest.x[i];
    }

    return r;
} /* md_32 */

/*
 * Return random unsigned 32-bit quantity. Use 'type' argument if
 * you need to generate several different values in close succession.
 */
uint32_t random32(int type)
{
    struct {
        int32_t type;
        struct timeval tv;
        clock_t cpu;
        pid_t pid;
        uint32_t hid;
        uid_t uid;
        gid_t gid;
        struct utsname name;
    } s;
    (void)gettimeofday(&s.tv, 0);
    (void)uname(&s.name);
    s.type = type;
    s.cpu = clock();
    s.pid = getpid();
    s.hid = gethostid();
    s.uid = getuid();
    s.gid = getgid();
    /* also: system uptime */

    return md_32((char *)&s, sizeof(s));
} /* random32 */


int main (void)
{
    uint32_t u32_rand_ts;
    uint16_t u16_rand_seq;
    int i;

    for (i = 0; i < 10; i++) {
        u32_rand_ts = random32(RANDOM_TIMESTAMP_TYPE);
        u16_rand_seq = random32(RANDOM_SEQUENCE_TYPE);

        printf("u32_rand_ts = %u, u16_rand_seq = %u\n", 
               u32_rand_ts, u16_rand_seq);
    }

    return 0;
}
