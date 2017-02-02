#include <string.h>

# define MONGO_likely(x) ((bool)(x))

#define MONGO_verify(_Expression) (void)( MONGO_likely(!!(_Expression)))

#if !defined(_WIN32)
    typedef int HANDLE;
    inline void strcpy_s(char *dst, unsigned len, const char *src) {
        MONGO_verify( strlen(src) < len );
        strcpy(dst, src);
    }
#else
    typedef void *HANDLE;
#endif


int main(void)
{
  return 0;
}
