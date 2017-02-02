#include <stdio.h>
#include <stdint.h>

int main(void)
{
    int i_ret, i_size = 88;
    uint32_t u_size = 88;

    i_ret = 88;
    printf("i_ret=%d (%u), u_size=%u, i_size=%d: \n\t(i_ret < u_size) is %d; "
           "(i_ret < i_size) is %d\n", 
           i_ret, (uint32_t)i_ret, u_size, i_size, 
           (i_ret < u_size), (i_ret < i_size));


    i_ret = 50;
    printf("i_ret=%d (%u), u_size=%u, i_size=%d: \n\t(i_ret < u_size) is %d; "
           "(i_ret < i_size) is %d\n", 
           i_ret, (uint32_t)i_ret, u_size, i_size, 
           (i_ret < u_size), (i_ret < i_size));


    i_ret = -1;
    printf("i_ret=%d (%u), u_size=%u, i_size=%d: \n\t(i_ret < u_size) is %d; "
           "(i_ret < i_size) is %d\n", 
           i_ret, (uint32_t)i_ret, u_size, i_size, 
           (i_ret < u_size), (i_ret < i_size));

    i_ret = -100;
    printf("i_ret=%d (%u), u_size=%u, i_size=%d: \n\t(i_ret < u_size) is %d; "
           "(i_ret < i_size) is %d\n", 
           i_ret, (uint32_t)i_ret, u_size, i_size, 
           (i_ret < u_size), (i_ret < i_size));

    
    return 0;
}
