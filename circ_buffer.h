/*------------------------------------------------------------------
 * Circular buffer - Header file
 *
 * Feb 2003, Lakshmi Narasimhan.S
 *
 * Copyright (c) 2003 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#ifndef _CIRC_BUFFER_H_
#define _CIRC_BUFFER_H_

#include "types.h"
#include "platform.h"
#include "master.h"

/*
 * Flags introduced for circular buffer
 */
#define CB_FLAGS_OVERWRITE      0x0001
#define CB_FLAGS_INTERRUPT_SAFE 0x0002
#define CB_FLAGS_USER           0x000F

#define CB_FLAGS_MALLOC         0x0010
#define CB_FLAGS_VALID          0x8000


struct cbuf_header_; 

typedef void    (*cbuf_cleanup_func_t)(void*);

typedef int (*cbuf_lookup_func_t)(void*, void*);

typedef boolean (*cbuf_enqueue_t)(struct cbuf_header_*, void*);
typedef void*   (*cbuf_dequeue_t)(struct cbuf_header_*);
typedef void*   (*cbuf_peekqueue_t)(struct cbuf_header_*);
typedef boolean (*cbuf_reset_t)(struct cbuf_header_*);
typedef void*   (*cbuf_lookup_t) (struct cbuf_header_*, void*, 
                                cbuf_lookup_func_t func);

typedef struct cbuf_action_t_ {
    cbuf_enqueue_t  enqueue;
    cbuf_dequeue_t  dequeue;
    cbuf_peekqueue_t pqueue;
    cbuf_reset_t    reset;
    cbuf_lookup_t   lookup;
} cbuf_action_t;

typedef struct cbuf_header_ {
    void *link;
    ulong *buffer;  /* the circular buffer as such */
    ulong *curr;    /* curr node in the circular buffer */
    ulong *base;    /* base node in the circular buffer */
    ushort flags;
    ulong count;   /* present number of entries */
    ulong max;     /* max number of entries */
    const char *name;
    cbuf_action_t *action;
    cbuf_cleanup_func_t cleanup_func;
} cbuf_header;



#define CB_VALID(flags) (flags & CB_FLAGS_VALID)
#define CB_EMPTY(hdr) (hdr->count == 0)
#define CB_FULL(hdr) (hdr->count == (hdr->max - 1))
#define CB_GET_DATA(node)       ((void*)(*node))
#define CB_SET_DATA(node, data) (*(node) = (ulong)data)
#define CB_COUNT(hdr) (hdr->count)

static inline void cbuf_interrupt_lock (ushort flags, leveltype *level)
{
    if (flags & CB_FLAGS_INTERRUPT_SAFE)
        *level = raise_interrupt_level(ALL_DISABLE);
}

static inline void cbuf_interrupt_unlock (ushort flags, leveltype *level)
{
    if (flags & CB_FLAGS_INTERRUPT_SAFE)
        reset_interrupt_level(*level);
}


/*
 * External API's.
 */
static inline boolean cbuf_enqueue (cbuf_header *chdr, void *data)
{
    return ((*chdr->action->enqueue)(chdr, data));
}

static inline void* cbuf_dequeue (cbuf_header *chdr)
{
    return ((*chdr->action->dequeue)(chdr));
}

static inline boolean cbuf_reset (cbuf_header *chdr)
{
    return ((*chdr->action->reset)(chdr));
}

static inline void* cbuf_peekqueue (cbuf_header *chdr)
{
    return ((*chdr->action->pqueue)(chdr));
}

static inline void* cbuf_lookup (cbuf_header *chdr, void *data, 
                               cbuf_lookup_func_t func)
{
    return ((*chdr->action->lookup)(chdr, data, func));
}

extern cbuf_header* cbuf_create(cbuf_header*, ushort, const char*, 
                            ushort, cbuf_cleanup_func_t);
extern boolean cbuf_destroy(cbuf_header*);
extern boolean cbuf_set_action(cbuf_header*, cbuf_action_t*);

#endif /* _CIRC_BUFFER_H_ */

