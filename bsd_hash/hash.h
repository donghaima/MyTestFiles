
/* 
 * Based on file search.h.
 */

#ifndef _HASH_H
#define	_HASH_H


/* Data type for reentrant functions */
struct hsearch_data
{
  struct internal_head *htable;
  size_t htablesize;
};


/* Reentrant versions which can handle multiple hashing tables at the same time */
int  new_hash_table(size_t nel, struct hsearch_data *htab);
void free_hash_table(struct hsearch_data *htab);
int  add_hash_entry(struct hsearch_data *htab, uint8_t *key, int key_len, void *data);
void delete_hash_entry(struct hsearch_data *htab, uint8_t *key, int key_len);
void *hash_lookup(struct hsearch_data *htab, uint8_t *key, int key_len);

#endif /* hash.h */
