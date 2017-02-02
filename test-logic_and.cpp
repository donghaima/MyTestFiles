#include <stdio.h>

bool f(void)
{
  return false;
}

int main(void)
{
    void* ptr = NULL;
    bool b = false;

    printf("ptr=%p, b=%d, ptr&& !f() = %d\n", ptr, b, ptr&&!f());
    return 0;
}
