#include <stdio.h>
#include <stdint.h>

typedef uint8_t boolean;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

inline boolean 
cmapi_arg_check_uint64 (uint64_t value, uint64_t min, uint64_t max)
{
    boolean result = TRUE;

    if ((value < min) || (value > max)) {
        result = FALSE;
    }
    return result;
}

inline boolean 
cmapi_arg_check_uint (uint32_t value, uint32_t min, uint32_t max)
{
    boolean result = TRUE;

    if ((value < min) || (value > max)) {
        result = FALSE;
    }
    return result;
}

#define CMAPI_ARG_CHECK_UINT(_v, _min, _max) \
cmapi_arg_check_uint(_v, _min, _max)


int main(void) 
{
    uint64_t myu64 = 0x01fffffffeLL;
    
    if (! CMAPI_ARG_CHECK_UINT(myu64, 0, 0xffffffff)) {
        printf("CHECK_UINT(0x%llx, 0, 0xffffffff) returns FALSE\n", myu64);
    } else {
        printf("CHECK_UINT(0x%llx, 0, 0xffffffff) returns TRUE\n", myu64);
    }

    if (! cmapi_arg_check_uint(myu64, 0, 0xffffffff)) {
        printf("check_uint(0x%llx, 0, 0xffffffff) returns FALSE\n", myu64);
    } else {
        printf("check_uint(0x%llx, 0, 0xffffffff) returns TRUE\n", myu64);
    }

    if (! cmapi_arg_check_uint64(myu64, 0, 0xffffffff)) {
        printf("check_uint64(0x%llx, 0, 0xffffffff) returns FALSE\n", myu64);
    } else {
        printf("check_uint64(0x%llx, 0, 0xffffffff) returns TRUE\n", myu64);
    }

    return 0;
}
