#include <stdio.h>
#include <stdint.h>

int main(void)
{
    uint64_t version = 0x1fffffffffffff00LL;
    char chan_name[] = "Channel 223.1.1.1:10004";

    /* Use a null format string */
    printf("");

    /* Not working */
    printf("Try to use binary format: 0b%b\n", 254);

    /* Correct one */
    printf("Correct: Version number is %llu for channel %s\n", 
           version, chan_name);

    /* This one happens to work fine */
    printf("Wrong but no core: Channel %s has version number %d\n", 
           chan_name, version);

    /* This one would segment fault */
    printf("cored: Version number is %d for channel %s\n", 
           version, chan_name);

    return 0;
}
