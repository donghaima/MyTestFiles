#include <stdio.h>
#include <stdint.h>

#define E1000_TYPE    1
#define IGB_TYPE      2
#define IXGBE_TYPE    3

/* Generate a uint32_t hardwareTypeAndIndex from one uint16_t hardwareType and 
 *  one uint16_t hardwareIndex:
 *          0..15 : adapter hardware index
 *         16..31 : adapter hardware type
 */
#define ADAPTER_HARDWARE_TYPE_AND_INDEX(t,i) ((uint32_t)(i) | ((t) << 16))

/* Get the adapter hardware type and index */
#define ADAPTER_HARDWARE_TYPE(ti)   ((uint16_t) (((ti) & 0xFFFF0000) >> 16))
#define ADAPTER_HARDWARE_INDEX(ti)  ((uint16_t) ((ti) & 0xFFFF))

int main (void)
{   
    uint16_t type = IXGBE_TYPE;
    uint16_t index;
    uint32_t ti;

    for (index=0; index<16; index++)
    {
        ti = ADAPTER_HARDWARE_TYPE_AND_INDEX(type, index);
        printf("type=%u, index=%u; ti=%u(0x%x); decoded type=%u index=%u.\n",
               type, index, ti, ti, ADAPTER_HARDWARE_TYPE(ti), ADAPTER_HARDWARE_INDEX(ti));
    }
    return 0;
}
