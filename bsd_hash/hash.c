
/* 
 * Reentrant hash table (thread unsafe though)
 *
 * Based on BSD hcreate_r.c.
 *  1. Added a new action REMOVE.
 *  2. Changed to use queue_plus.h from VAM project.
 */


#include "queue_plus.h"   /* VAM */
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "hash.h"

/*
 * DO NOT MAKE THIS STRUCTURE LARGER THAN 32 BYTES (4 ptrs on 64-bit
 * ptr machine) without adjusting MAX_BUCKETS_LG2 below.
 */
struct internal_entry {
    SLIST_ENTRY(internal_entry) link;
    uint8_t *key;
    int      key_len;
    void    *data;                           
};
SLIST_HEAD(internal_head, internal_entry);

#define	MIN_BUCKETS_LG2	4
#define	MIN_BUCKETS	(1 << MIN_BUCKETS_LG2)

/*
 * max * sizeof internal_entry must fit into size_t.
 * assumes internal_entry is <= 32 (2^5) bytes.
 */
#define	MAX_BUCKETS_LG2	(sizeof (size_t) * 8 - 1 - 5)
#define	MAX_BUCKETS	((size_t)1 << MAX_BUCKETS_LG2)


/* ISC hash function */
static inline uint32_t hash (uint8_t *key, int key_len, int htbl_size)
{
    register int accum = 0;
    register uint8_t *s = key;
    int i = key_len;
    while (i--) {
        /* Add the character in... */
        accum += *s++;
        /* Add carry back in... */
        while (accum > 255) {
            accum = (accum & 255) + (accum >> 8);
        }
    }
    return accum % htbl_size;
}



/* Create a hash table of size nel */
int new_hash_table(size_t nel, struct hsearch_data *htab)
{
    size_t idx;
    unsigned int p2;

    /* Make sure this this isn't called when a table already exists. */
    if (htab->htable != NULL) {
        errno = EINVAL;
        return 0;
    }

    /* If nel is too small, make it min sized. */
    if (nel < MIN_BUCKETS)
        nel = MIN_BUCKETS;

    /* If it's too large, cap it. */
    if (nel > MAX_BUCKETS)
        nel = MAX_BUCKETS;
    
    /* If it's is not a power of two in size, round up. */
    if ((nel & (nel - 1)) != 0) {
        for (p2 = 0; nel != 0; p2++)
            nel >>= 1;
        nel = 1 << p2;
    }
	
    /* Allocate the table.
       TODO: change to use zone malloc later. */
    htab->htablesize = nel;
    htab->htable = malloc(htab->htablesize * sizeof htab->htable[0]);
    if (htab->htable == NULL) {
        errno = ENOMEM;
        return 0;
    }
    
    /* Initialize it. */
    for (idx = 0; idx < htab->htablesize; idx++)
        SLIST_INIT(&(htab->htable[idx]));

    return 1;
}

/* Destroy the specified hash table: before destroying a hash table,
 * make sure to clean up the data pointed by the hash entries.
 */
void free_hash_table(struct hsearch_data *htab)
{
    struct internal_entry *ie;
    size_t idx;

    if (htab->htable == NULL)
        return;

    free(htab->htable);
    htab->htable = NULL;
}


/* Add an entry (key, data) to hash table htab: assuming the key is not in the hash table
 * Note: key octet string must have been allocated from heap before call this.
 */
int add_hash_entry(struct hsearch_data *htab, uint8_t *key, int key_len, void *data)
{
    struct internal_head *head;
    struct internal_entry *ie;
    uint32_t hashval;
    size_t len;

    if (!htab) {
        printf("htab is NULL\n");
        return 0;
    }
    
    if (!key_len) {
        key_len = strlen(key);
    }

    hashval = hash(key, key_len, htab->htablesize); 
    head = &(htab->htable[hashval & (htab->htablesize - 1)]);

    /* Add a new entry */
    ie = malloc(sizeof *ie);
    if (ie == NULL)
    {
        return 0;
    }
    ie->key = key;
    ie->key_len = key_len;
    ie->data = data;

    /* Add the entry to the collisio list */
    SLIST_INSERT_HEAD(head, ie, link);
    return 1;
}


void delete_hash_entry(struct hsearch_data *htab, uint8_t *key, int key_len)
{

    struct internal_head *head;
    struct internal_entry *ie, *prev_ie = NULL;
    uint32_t hashval;
    size_t len;

    if (!htab) {
        printf("htab is NULL\n");
        return;
    }
    
    if (!key_len) {
        key_len = strlen(key);
    }

    hashval = hash(key, key_len, htab->htablesize); 
    head = &(htab->htable[hashval & (htab->htablesize - 1)]);

    /* Go through the list looking for an entry that matches;
       if we find it, delete it. */
    ie = SLIST_FIRST(head);
    while (ie != NULL) {
        if (key_len == ie->key_len && (memcmp(ie->key, key, key_len) == 0)) {
            /* Maintain the collision list */
            if (prev_ie)
                SLIST_NEXT(prev_ie, link) = SLIST_NEXT(ie, link);
            else 
                SLIST_FIRST(head) = SLIST_NEXT(ie, link);
            
            free(ie);
            break;
        }
        prev_ie = ie;
        ie = SLIST_NEXT(ie, link);
    }
}


/* Lookup an entity from the hash table */
void *hash_lookup(struct hsearch_data *htab, uint8_t *key, int key_len)
{
    struct internal_head *head;
    struct internal_entry *ie;
    uint32_t hashval;
    size_t len;

    if (!htab) {
        printf("htab is NULL\n");
        return 0;
    }
    
    if (!key_len) {
        key_len = strlen(key);
    }
    
    //hashval = hash(key, key_len);
    hashval = hash(key, key_len, htab->htablesize); 
    
    head = &(htab->htable[hashval & (htab->htablesize - 1)]);
    ie = SLIST_FIRST(head);
    while (ie != NULL) {
        if (key_len == ie->key_len && (memcmp(ie->key, key, key_len) == 0))
            break;
        ie = SLIST_NEXT(ie, link);
    }

    return (ie != NULL) ? ie->data : NULL;
}


