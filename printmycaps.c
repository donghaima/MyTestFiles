#include <stdio.h>
#include <sys/prctl.h>
#include <sys/capability.h>
#include <sys/types.h>

int main (void)
{
    cap_t cap = cap_get_proc();

    printf("Running with uid %d\n", getuid());
    printf("Running with capabilities: %s\n", cap_to_text(cap, NULL));

    cap_free(cap);

    return 0;
}
