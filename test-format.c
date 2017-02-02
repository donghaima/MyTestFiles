#include <stdio.h>
#include <stdint.h>

int main(void)
{
    int64_t i64 = 64;
    int32_t i32 = 32;
    printf("i64=%lld, i32=%d, sizeof(long)=%lu, sizeof(long long)=%lu\n",
           i64, i32, sizeof(long), sizeof(long long));

    return 0;
}
