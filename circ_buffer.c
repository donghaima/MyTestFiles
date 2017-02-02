/*------------------------------------------------------------------
 * Circular Buffer - Library Routines.
 *
 * Feb 2003, Lakshmi Narasimhan.S
 *
 * Copyright (c) 2003-2004 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include COMP_INC(kernel/memory, free.h)
#include COMP_INC(kernel, printf.h)
#include COMP_INC(kernel, ios_kernel_debug.h)
#include "circ_buffer.h"

static queuetype cbufQ;

/*
 * Note: Until the debug.h file is cleaned up,
 * we can not include os_debug_flags.h.  So,
 * extern the variable we need.
 */
extern boolean sanity_debug;

#define cbuf_state_buginf   if (sanity_debug || cbuf_state_debug) \
                                (*kernel_buginf)
static boolean cbuf_state_debug = FALSE;


static boolean cbuf_enqueue_default (cbuf_header *chdr, void *data)
{
    ushort flags; 
    leveltype level;
    void *chdata;

    flags = chdr->flags;
    cbuf_interrupt_lock(flags, &level);
    if (!CB_VALID(flags)) {
        cbuf_state_buginf("***Circular Buffer Enqueue called "
                        "for invalid data****\n");
        cbuf_interrupt_unlock(flags, &level);
        return (FALSE);
    }

    if ((CB_FULL(chdr) && (chdr->flags & CB_FLAGS_OVERWRITE)) ||
        (!CB_FULL(chdr))) {
        CB_SET_DATA(chdr->curr, data);
        chdr->count++;
        chdr->curr++;
        if (chdr->curr >= &chdr->buffer[chdr->max]) {
            chdr->curr = chdr->buffer;
        }
        if (chdr->curr == chdr->base) {
            /*
             * First free the data pointed to by base.
             */
            if (chdr->cleanup_func) {
                chdata = CB_GET_DATA(chdr->base);
                (*chdr->cleanup_func)(chdata);
            }
            chdr->base++;
            chdr->count--;
            if (chdr->base >= &chdr->buffer[chdr->max]) {
                chdr->base = chdr->buffer;
            }
        }
    }
    cbuf_interrupt_unlock(flags, &level);
    return (TRUE);
}

static void* cbuf_dequeue_default (cbuf_header *chdr)
{
    ushort flags;
    leveltype level;
    void *data;

    flags = chdr->flags;
    cbuf_interrupt_lock(flags, &level);
    if (!CB_VALID(flags)) {
        cbuf_state_buginf("***Circular Buffer Dequeue called for "
                        "invalid data***\n");
        cbuf_interrupt_unlock(flags, &level);
        return (NULL);
    }
    if (CB_EMPTY(chdr)) {
        cbuf_interrupt_unlock(flags, &level);
        return (NULL);
    }

    data = CB_GET_DATA(chdr->base);
    chdr->count--;
    chdr->base++;
    if (chdr->base >= &chdr->buffer[chdr->max]) {
        chdr->base = chdr->buffer;
    }
    /*
     * A simple sanity check here!
     */
    if (chdr->base == chdr->curr) {
        if (chdr->count != 0) {
           cbuf_state_buginf("***Circular buffer empty but count "
                           "not zero***\n");
           chdr->count = 0;
        }
    }
    cbuf_interrupt_unlock(flags, &level);
    return (data);
}


static void* cbuf_peekqueue_default (cbuf_header *chdr)
{
    leveltype level;
    ushort flags;
    void *data;

    flags = chdr->flags;
    if (!CB_VALID(flags)) {
        cbuf_state_buginf("***Circular buffer peekqueue called with "
                          "invalid data***\n");
        return (NULL);
    }
    if (CB_EMPTY(chdr)) {
        return (NULL);
    } 
    cbuf_interrupt_lock(flags, &level);
    data = CB_GET_DATA(chdr->base);
    cbuf_interrupt_unlock(flags, &level);
    return (data);
}

static boolean cbuf_reset_default (cbuf_header *chdr)
{
    leveltype level;
    ushort flags;
    void *cbdata;
    ulong *runner;

    flags = chdr->flags;
    if (!CB_VALID(flags)) {
        cbuf_state_buginf("***Circular buffer cleanup called with "
                        "invalid data***\n");
        return (FALSE);                                      
    }

    if (CB_EMPTY(chdr) || !chdr->cleanup_func) {
        chdr->base = chdr->curr = chdr->buffer;
        chdr->count = 0;
        return (TRUE);
    }
    cbuf_interrupt_lock(flags, &level);
    runner = chdr->base;
    while (runner != chdr->curr) {
        cbdata = CB_GET_DATA(runner);
        (*chdr->cleanup_func)(cbdata);
        runner++;
        if (runner >= &chdr->buffer[chdr->max]) {
            runner = chdr->buffer;
        }
    }

    chdr->base = chdr->curr = chdr->buffer;
    chdr->count = 0;
    cbuf_interrupt_unlock(flags, &level);
    return (TRUE);
}

static void * cbuf_lookup_default (cbuf_header *chdr, void* data, 
                                 cbuf_lookup_func_t func) 
{
    leveltype level;
    ushort flags;
    void *cbdata;
    ulong *runner;

    flags = chdr->flags;
    if (!CB_VALID(flags)) {
        cbuf_state_buginf("***Circular buffer lookup called with "
                        "invalid data***\n");
        return (NULL);
    }

    if (CB_EMPTY(chdr)) {
        return (NULL);
    }

    cbuf_interrupt_lock(flags, &level);

    runner = chdr->base;
    while (runner != chdr->curr) {
        cbdata = CB_GET_DATA(runner);
        if ((*func)(cbdata, data) == 0) {
            cbuf_interrupt_unlock(flags, &level);
            return (cbdata);
        }
        runner++;
        if (runner >= &chdr->buffer[chdr->max]) {
            runner = chdr->buffer;
        }
    }
    cbuf_interrupt_unlock(flags, &level);
    return (NULL);
}

/*
 * default circular buffer action vectors.
 */
static cbuf_action_t cbuf_action_vectors_default = {
    cbuf_enqueue_default,
    cbuf_dequeue_default,
    cbuf_peekqueue_default,
    cbuf_reset_default,
    cbuf_lookup_default,
};

boolean cbuf_set_action (cbuf_header *chdr, cbuf_action_t *action)
{
    /*
     * Make sure we're sane
     */
    if (!chdr)
        return(FALSE);

    /*
     * Organic fix. If someone specifies NULL as the action vector
     * pointer, we use the default vector block.
     */
    if (!action)
        action = &cbuf_action_vectors_default;
    else {
        if (!action->enqueue)
            action->enqueue = cbuf_action_vectors_default.enqueue;
        if (!action->dequeue)
            action->dequeue = cbuf_action_vectors_default.dequeue;
        if (!action->lookup)
            action->lookup = cbuf_action_vectors_default.lookup;
    }
    /*
     * Set our vector
     */
    chdr->action = action;

    return(TRUE);
}


cbuf_header *cbuf_create (cbuf_header *chdr, ushort max, 
                      const char *name, ushort flags, 
                      cbuf_cleanup_func_t clfunc)
{
    uint cmax;
    ulong *cbuffer;

    /*
     * A circular buffer should have the maximum number of entries specified.
     */
    if (max == 0) {
        return (NULL);
    }

    if (!chdr) {
        /*
         * The calling function was cheap and wants us to provide
         * the space for the list structure. Be obliging.
         * Lets think about chunks later...
         */
        chdr = malloc(sizeof(cbuf_header)); 

        /*
         * We asked, but none was given. Oh well...
         */
        if (!chdr) {
            return(NULL);
        }

        chdr->flags |= CB_FLAGS_MALLOC;
    } else {
        /*
         * Ensure that the slate is clean before starting.
         */
        memset(chdr, 0, sizeof(cbuf_header));
    }


    cmax = max + 1;
    chdr->max = cmax;
    chdr->count = 0;

    chdr->name = (name != NULL) ? name : "(unknown)";
    chdr->flags |= (flags & CB_FLAGS_USER);
    chdr->cleanup_func = clfunc;
    
    cbuffer = malloc(cmax * sizeof(void*));
    if (!cbuffer) {
        free(chdr);
        return (NULL);
    }
    chdr->buffer = cbuffer;
    chdr->curr = cbuffer;
    chdr->base = cbuffer;
    p_enqueue(&cbufQ, chdr);

    /*
     * Set the default vectors
     */
    cbuf_set_action(chdr, NULL);
    chdr->flags |= CB_FLAGS_VALID;
  
    return (chdr);
}

boolean cbuf_destroy (cbuf_header *chdr)
{
    ulong *runner;
    void *cbdata;
    leveltype level;
    ushort flags;

    flags = chdr->flags;
    cbuf_interrupt_lock(flags, &level);

    if (!CB_EMPTY(chdr) && chdr->cleanup_func) {
        runner = chdr->base;
        while (runner != chdr->curr) {
            cbdata = CB_GET_DATA(runner);
            (*chdr->cleanup_func)(cbdata);
            runner++;
            if (runner >= &chdr->buffer[chdr->max]) {
                runner = chdr->buffer;
            }
        }
    }

    chdr->flags &= ~CB_FLAGS_VALID;
    /*
     * Remove from the global Q.
     */
    p_unqueue(&cbufQ, chdr);

    /*
     * Clean up the buffer.
     */
    free(chdr->buffer);
    chdr->buffer = NULL;
    if (chdr->flags & CB_FLAGS_MALLOC) {
        free(chdr);
    }
    cbuf_interrupt_unlock(flags, &level);
    return (TRUE);
}

