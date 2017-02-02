#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef struct test_struct_ {
    uint64_t u64_a;
    uint64_t u64_b;
    uint64_t u64_c;

    uint32_t u32_a;
    uint32_t u32_b;
    uint32_t u32_c;

    char name[64];
    void *ptr;

    struct key_t {
        int k;
    } key;
} test_struct_t;

int main(void)
{
	int i = 10;
	int *p_i = NULL, **pp_i = NULL;
        const char * encap_string = NULL;

	//printf("i = %d, *p_i=%d, *pp_i=%x\n", i, *p_i, *pp_i);

        encap_string = "encap_string";
	printf("encap_string=%p, %s\n", encap_string, encap_string);

        test_struct_t struct1, struct2;
        
        struct2.u64_a = 123456789;
        struct2.u64_b = 987654321;
        struct2.u64_c = 0;

        struct2.u32_a = 54321;
        struct2.u32_b = 12345;
        struct2.u32_c = 22222;

        strncpy(struct2.name, "test struct assignment", 64);
        struct2.ptr = &struct2;

        struct2.key.k = 88888;

        struct1 = struct2;

	return;
}
