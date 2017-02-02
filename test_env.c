#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <environment var>\n", argv[0]);
        return -1;
    }

    char *name = argv[1];
    char *value = NULL;

    value = getenv(name);
    printf("Got env variable %s = \"%s\"\n", name, value);

    return 0;
}
