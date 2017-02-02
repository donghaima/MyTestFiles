#include <stdio.h>
#include <stdint.h>

int main (void)
{
        int64_t tmpval;
        char buf[64];

        int num = sscanf(buf, "%lld", &tmpval);
        if (num == 1) {
            printf("Read %ld\n", tmpval);
            return 0;
        }

        return 1;
}
