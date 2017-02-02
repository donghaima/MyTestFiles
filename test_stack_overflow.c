#include <string.h>
#include <stdint.h>
#include <stdio.h>
 
void foo (char *bar)
{
    char c[12];
    memcpy(c, bar, strlen(bar));  // no bounds checking...

}
 
int main (int argc, char **argv)
{
   foo(argv[1]); 
}

#if 0
http://en.wikipedia.org/wiki/Stack_buffer_overflow

Try to run:
	1. ./test_stack_overflow "hello"
	2. ./test_stack_overflow "AAAAAAAAAAAAAAAAAAAA\x08\x35\xC0\x80"
        3. ./test_stack_overflow "12345642894294820482982493"
#endif

