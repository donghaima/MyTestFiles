#include <stdio.h>
#include <stdint.h>

typedef struct rtcptype_ {
    /*
     * Keep the fields in a single ushort and do an ntohs to avoid
     * little/big endian checks.
     */
    uint16_t params;        /* version:2 padding:1 count:5 packet type:8 */
    uint16_t len;                /* length of message in long words */

    /*
     * Synchronization src id. Could be the sender's for source report,
     * or the receiver's for reception report.
     */
    uint32_t  ssrc;       
} rtcptype;


int main (void)
{
    char buf[1472] = {0};
    
    char addr_str[12];
    char cname[256];

    char msg_buf[256], msg_buf2[256];

    rtcptype *p_rtcp = (rtcptype *)buf;

    rtcptype *p_rtcp2 = (rtcptype *)&buf[0];


    return 0;
}
