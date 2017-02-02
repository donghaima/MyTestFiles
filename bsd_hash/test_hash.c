#include <netinet/in.h>
#include <stdio.h>

#include "hash.h"

/* Different type of key structure */
typedef struct my_key_t_ {
    in_addr_t addr;
    in_port_t port;
} my_key_t;


int main() {
    struct hsearch_data htab, htab2;
    int i, r;
    in_addr_t start_addr = 0x0a0a0101;
    in_port_t start_port = 0xc350;
    uint8_t *key;
    uint32_t search_key;
    void *data = NULL;
    
    /* Generating some keys */
    my_key_t keys[40];
    int data_int[40];

    for (i = 0; i < 40; i++) {
        keys[i].addr = start_addr + i;
        keys[i].port = start_port + i;
        data_int[i] = i;
        printf("Test data (i=%d): addr=0x%x, .port=%d - key=0x%x - data_int[i]=%d\n",
               i, keys[i].addr, keys[i].port, 
               ntohl(keys[i].addr) ^ keys[i].port, data_int[i]);
    }
    
    memset((void *)&htab, 0, sizeof(htab));

    printf("\nCreate the first htable: htab\n");

    /* Starting with small table, and letting it grow does not work */
    r = new_hash_table(sizeof(keys), &htab);
    if (r == 0) {
        fprintf(stderr, "create failed\n");
        exit(1);
    }

    for (i = 0; i < 20; i++) {
        key = malloc(sizeof(uint32_t));
        uint32_t k = ntohl(keys[i].addr) ^ keys[i].port;
        memcpy (key, &k, sizeof(uint32_t));

        r = add_hash_entry(&htab, key, sizeof(uint32_t), (void *)&data_int[i]);

        /* there should be no failures */
        if (!r) {
            fprintf(stderr, "add_hash_entry failed\n");
            exit(1);
        }
    }

    /* Delete the entries from 10 to 15 */
    for (i = 10; i < 15; i++) {
        uint32_t k = ntohl(keys[i].addr) ^ keys[i].port;
        memcpy(&search_key, &k, sizeof(uint32_t));

        delete_hash_entry(&htab, (uint8_t *)&search_key, sizeof(search_key));
    }


    for (i = 0; i < 26; i++) {
        /* print two entries from the table, and
           show that two are not in the table */
        uint32_t k = ntohl(keys[i].addr) ^ keys[i].port;
        memcpy(&search_key, &k, sizeof(uint32_t));

        data = NULL;
        data = hash_lookup(&htab, (uint8_t *)&search_key, sizeof(uint32_t));
        if (data) {
            int data_int = *(int *)data;
            printf("htab: i=%d, key=0x%x -> %d\n", i, key, data_int);
        } else {
            printf("htab: i=%d, key=0x%x not in hash table\n", i, key);
        }
    }

    
    /* Now create a second hash table */
    printf("\nNow create a seond htable: htab2\n");
    memset((void *)&htab2, 0, sizeof(htab2));
    
    r = new_hash_table(sizeof(keys), &htab2);
    if (r == 0) {
        fprintf(stderr, "create failed\n");
        exit(1);
    }

    printf("\nhtab2: Add entries from 10 to 30\n");
    for (i = 10; i < 30; i++) {
        key = malloc(sizeof(uint32_t));
        uint32_t k = ntohl(keys[i].addr) ^ keys[i].port;
        memcpy (key, &k, sizeof(uint32_t));

        r = add_hash_entry(&htab2, key, sizeof(uint32_t), (void *)&data_int[i]);

        /* there should be no failures */
        if (!r) {
            fprintf(stderr, "add_hash_entry failed\n");
            exit(1);
        }
    }

    printf("\nhtab2: Now delete entries from 15 to 20\n");
    /* Delete the entries from 15 to 20 */
    for (i = 15; i < 20; i++) {
        uint32_t k = ntohl(keys[i].addr) ^ keys[i].port;
        memcpy(&search_key, &k, sizeof(uint32_t));

        delete_hash_entry(&htab2, (uint8_t *)&search_key, sizeof(search_key));
    }

    printf("\nhtab2: Print entries from 10 to 40\n");
    for (i = 10; i < 40; i++) {
        /* print two entries from the table, and
           show that two are not in the table */
        uint32_t k = ntohl(keys[i].addr) ^ keys[i].port;
        memcpy(&search_key, &k, sizeof(uint32_t));

        data = NULL;
        data = hash_lookup(&htab2, (uint8_t *)&search_key, sizeof(uint32_t));
        if (data) {
            int data_int = *(int *)data;
            printf("htab2: i=%d, key=0x%x -> %d\n", i, key, data_int);
        } else {
            printf("htab2: i=%d, key=0x%x not in hash table\n", i, key);
        }
    }

    printf("\nhtab2: Now add back entries from 15 to 20\n");
    /* Add back the entries from 15 to 20 */
    for (i = 15; i < 20; i++) {
        //uint32_t k = ntohl(keys[i].addr) ^ keys[i].port;
        //memcpy(&search_key, &k, sizeof(uint32_t));

        key = malloc(sizeof(uint32_t));
        uint32_t k = ntohl(keys[i].addr) ^ keys[i].port;
        memcpy (key, &k, sizeof(uint32_t));

        r = add_hash_entry(&htab2, key, sizeof(uint32_t), (void *)&data_int[i]);

        /* there should be no failures */
        if (!r) {
            fprintf(stderr, "add_hash_entry failed\n");
            exit(1);
        }
    }

    printf("\nhtab2: Print entries from 10 to 40 again\n");
    for (i = 10; i < 40; i++) {
        /* print two entries from the table, and
           show that two are not in the table */
        uint32_t k = ntohl(keys[i].addr) ^ keys[i].port;
        memcpy(&search_key, &k, sizeof(uint32_t));

        data = NULL;
        data = hash_lookup(&htab2, (uint8_t *)&search_key, sizeof(uint32_t));
        if (data) {
            int data_int = *(int *)data;
            printf("htab2: i=%d, key=0x%x -> %d\n", i, key, data_int);
        } else {
            printf("htab2: i=%d, key=0x%x not in hash table\n", i, key);
        }
    }

    /* */

    return 0;
}
