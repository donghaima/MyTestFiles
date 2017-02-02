#include <stdio.h>

#define __tst_rpc_id_strings__ \
    "INVALID", \
    "my_foo", \
    "my_bar"


int main (void) 
{
    printf("string=%s\n", __tst_rpc_id_strings__);

    return 0;
}
