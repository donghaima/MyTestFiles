#include <string.h>

const char *a( void )
{
        return "abc";
}


char *b( void )
{
        return "abc";
}

int main( void )
{
        //char *s = a();   /* This line will generate a compilation warning */
	char *file = strrchr(__FILE__, '/'); /* */
        const char *s2 = __FILE__;
	const char *r = b();

        return 0;
}

