#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

int main(void)
{
    int is_true=0, on_wheel=1;

    //while (! on_wheel && is_true) {
    while (! is_true && on_wheel) {
        printf("in loop\n");
        sleep(1);
    }

    return 0;
}
