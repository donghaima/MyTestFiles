#include <stdio.h>
#include <stdint.h>

int main(void)
{
    uint64_t val = 0;
    uint32_t chan_ip = inet_addr("225.3.15.78");
    uint16_t chan_port = htons(50000);

    val = (uint64_t)chan_ip<<16 | chan_port;
	
	uint32_t u32_tmp;
	u32_tmp = (uint32_t)val;  // test casting
    printf("Channel ipaddr=0x%x port=0x%x; val=0x%llx; u32_tmp=0x%x\n",
           chan_ip, chan_port, val, u32_tmp);

    
    uint32_t stb_ip = inet_addr("12.34.56.78");
    val = stb_ip;
    printf("STB ipaddr=0x%x; val=0x%llx\n", stb_ip, val);


    return 0;
}
