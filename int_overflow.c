#include <stdio.h>
#include <stdint.h>

#define E_FAIL        (0x80004005L)
#define S_OK     (0)

int32_t main (void)
{
    int32_t ret = S_OK;
    ret = E_FAIL;
    printf("ret = 0x%x\n", ret);
    return ret;
}
