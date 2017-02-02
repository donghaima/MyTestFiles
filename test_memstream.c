/* ./a.out "1 23 43"
 * size=11; ptr=1 529 1849
 */

#define _GNU_SOURCE
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    FILE *out, *in;
    int v, s;
    size_t size = 0;
    char *ptr;
    assert(argc == 2);
    in = fmemopen(argv[1], strlen(argv[1]), "r");
    if (in == NULL) { perror("fmemopen"); exit(EXIT_FAILURE);}
    out = open_memstream(&ptr, &size);
    if (out == NULL) { perror("fmemopen"); exit(EXIT_FAILURE);}
    for (;;) {
        s = fscanf(in, "%d", &v);
        if (s <= 0)
            break;
        s = fprintf(out, "%d ", v * v);
        if (s == -1) { perror("fprintf"); exit(EXIT_FAILURE); }
    }
    fclose(in);
    fclose(out);
    printf("size=%ld; ptr=%s\n", (long) size, ptr);
    free(ptr);
    exit(EXIT_SUCCESS);
}
