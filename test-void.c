#include <stdio.h>
#include <stdlib.h>

int main (void)
{
    int *pInt = new int(100 * sizeof(int));
    void* pVoid = (void*)pInt;

    printf("sizeof(pInt):%d; sizeof(pVoid):%d\n", sizeof(pInt), sizeof(pVoid));    
    printf("sizeof(*pInt):%d; sizeof(*pVoid):%d\n", sizeof(*pInt), sizeof(*pVoid));

    delete(pVoid);

    return 0;
}
