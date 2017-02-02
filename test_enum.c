#include <stdio.h>
#include <stdint.h>

typedef enum cm_rx_encap_
{
    /**@brief 
     * UDP encapsulation */
    CMENCAP_UDP, 

    /**@brief 
     * RTP encapsulation */
    CMENCAP_RTP,

    /**@brief 
     * Must always be last enum value */
    CMENCAP_NUM_VAL

} cm_rx_encap_t;


#define __HALF_MAX_SIGNED(type) ((type)1 << (sizeof(type)*8-2))
#define __MAX_SIGNED(type) (__HALF_MAX_SIGNED(type) - 1 + \
                            __HALF_MAX_SIGNED(type))
#define __MIN_SIGNED(type) (-1 - __MAX_SIGNED(type))

#define __MIN(type) ((type)-1 < 1?__MIN_SIGNED(type):(type)0)
#define __MAX(type) ((type)~__MIN(type))


int main(void)
{
    cm_rx_encap_t myenum = CMENCAP_RTP;

    printf("myenum=%d, size=%d\n", myenum, sizeof(myenum));

    printf("(cm_rx_encap_t)-1 = %d(0x%x)\n", 
           (cm_rx_encap_t)-1, (cm_rx_encap_t)-1);
    printf("(uint16_t)-1 = %u(0x%x)\n", 
           (uint16_t)-1, (uint16_t)-1);
    printf("(uint32_t)-1 = %u(0x%x)\n", 
           (uint32_t)-1, (uint32_t)-1);
    printf("(uint64_t)-1 = %llu(0x%llx)\n", 
           (uint64_t)-1, (uint64_t)-1);


    printf("\nenum: MIN=%d, MAX=%d\n", 
           __MIN(cm_rx_encap_t), __MAX(cm_rx_encap_t));
    printf("\nint: MIN=%d, MAX=%d\n", __MIN(int), __MAX(int));
    printf("int8_t: MIN=%d, MAX=%d\n", __MIN(int8_t), __MAX(int8_t));
    printf("int16_t: MIN=%d, MAX=%d\n", __MIN(int16_t), __MAX(int16_t));
    printf("int32_t: MIN=%d, MAX=%d\n", __MIN(int32_t), __MAX(int32_t));

    printf("\nuint8_t: MIN=%u, MAX=%u\n", __MIN(uint8_t), __MAX(uint8_t));
    printf("uint16_t: MIN=%u, MAX=%u\n", __MIN(uint16_t), __MAX(uint16_t));
    printf("uint32_t: MIN=%u, MAX=%u\n", __MIN(uint32_t), __MAX(uint32_t));
    printf("uint64_t: MIN=%llu, MAX=%llu\n", __MIN(uint64_t), __MAX(uint64_t));


    return 0;
}
