/* $Id: $
 * $Source: $
 *------------------------------------------------------------------
 * bitlist.c - dynamic bitlists
 *
 * November 1998, Steve Larson
 * 
 * Copyright (c) 1998-2003, 2005-2007 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 * $Log: $
 *------------------------------------------------------------------
 * $Endlog$
 */

#include "master.h"
#include "string.h"
#include "stdlib.h"
#define DEFINE_MESSAGES TRUE
#include "logger.h"
#include "msg_bitlist.c"
#include "bitlist.h"

boolean
bitlist_validbit(bitlist_t *bl, int bit)
{
    if (bit < 0 || bit >= bl->state.max_bits) {
	errmsg(&msgsym(OUTOFRANGE, BIT),
               bit + bl->state.base_bit,
               bl->state.base_bit,
               bl->state.base_bit + bl->state.max_bits - 1);
	return FALSE;
    }
    return TRUE;
}

boolean
bitlist_validrange(bitlist_t *bl, int bit, int bits)
{
    if (!bitlist_validbit(bl, bit-(bl->state.base_bit)))
	return FALSE;
    
    if (!bitlist_validbit(bl, bit+bits-(bl->state.base_bit)-1))
	return FALSE;
    
    return TRUE;
}

boolean
bitlist_test(bitlist_t *bl, int bit)
{
    bit -= bl->state.base_bit;
    if (!bitlist_validbit(bl, bit))
	return FALSE;
    if (bl->bits[BIT_TO_ELEM(bit)] & BIT_TO_MASK(bit))
	return TRUE;
    return FALSE;
}

void
bitlist_set(bitlist_t *bl, int bit)
{
    bit -= bl->state.base_bit;
    if (bitlist_validbit(bl, bit))
	bl->bits[BIT_TO_ELEM(bit)] |= BIT_TO_MASK(bit);
}

void
bitlist_clear(bitlist_t *bl, int bit)
{
    bit -= bl->state.base_bit;
    if (bitlist_validbit(bl, bit))
	bl->bits[BIT_TO_ELEM(bit)] &= ~BIT_TO_MASK(bit);
}

void
bitlist_setall(bitlist_t *bl, int bits, int base)
{
    bl->state.max_elems = BITS_TO_ELEMS(bits);
    bl->state.max_bits = bits;
    bl->state.base_bit = base;
    bl->state.last_elem = 0;
    bl->state.last_bit = 0;

    memset(&bl->bits[0], ~0, bl->state.max_elems * sizeof(bitlist_elem_t));

    /*
    ** Clear overflow bits in last element
    */
    if (BIT_TO_BIT(bits))
        bl->bits[bl->state.max_elems - 1] &= BIT_TO_MASK(bits) - 1;
}

void
bitlist_set_contiguous(bitlist_t *bl, int bit, int bits)
{
    if (!bitlist_validrange(bl, bit, bits))
	return;
    
    /*
     * Only set if the starting bit is an exact multiple of 32  
     */
    if (((bit-(bl->state.base_bit))%BITS_PER_ELEM) != 0) 
        return;
    
    /*
     * Only set if the number of bits to copy is an exact multiple of 32  
     */
    if ((bits%BITS_PER_ELEM) != 0)
        return;

    memset(&bl->bits[BIT_TO_ELEM(bit-(bl->state.base_bit))], ~0, 
           BITS_TO_ELEMS(bits) * sizeof(bitlist_elem_t));
}

void
bitlist_clearall(bitlist_t *bl, int bits, int base)
{
    bl->state.max_elems = BITS_TO_ELEMS(bits);
    bl->state.max_bits = bits;
    bl->state.base_bit = base;
    bl->state.last_elem = 0;
    bl->state.last_bit = 0;

    memset(&bl->bits[0], 0, bl->state.max_elems * sizeof(bitlist_elem_t));
}

void
bitlist_clear_contiguous(bitlist_t *bl, int bit, int bits)
{
    if (!bitlist_validrange(bl, bit, bits))
	return;
    
    /*
     * Only clear if the starting bit is an exact multiple of 32  
     */
    if (((bit-(bl->state.base_bit))%BITS_PER_ELEM) != 0)
        return;
    
    /*
     * Only clear if the number of bits to copy is an exact multiple of 32  
     */
    if ((bits%BITS_PER_ELEM) != 0)
        return;

    memset(&bl->bits[BIT_TO_ELEM(bit-(bl->state.base_bit))], 0, 
           BITS_TO_ELEMS(bits) * sizeof(bitlist_elem_t));
    
}

void
bitlist_clearall2(bitlist_t *bl)
{
    memset(&bl->bits[0], 0, bl->state.max_elems * sizeof(bitlist_elem_t));
}

int
bitlist_find_bit(bitlist_t *bl, boolean first)
{
    bitlist_elem_t   bits;
    int              elem, bit;

    if (first) {
	elem = 0;
	bit = 0;
    } else {
	elem = bl->state.last_elem;
	if (elem >= bl->state.max_elems)
	    return (BITLIST_INVALID);
	bit = bl->state.last_bit + 1;
    }
    for (; elem < bl->state.max_elems; elem++, bit = 0) {
	if (bit >= BITS_PER_ELEM)
	    continue;
	if ((bits = bl->bits[elem]) == 0)
            continue;
	while (bit < BITS_PER_ELEM) {
	    if (bits & (1 << bit))
		goto found;
	    bit++;
	}
    }
found:
    bl->state.last_elem = elem;
    bl->state.last_bit = bit;
    if (elem >= bl->state.max_elems)
	return (BITLIST_INVALID);
    bit = (elem * BITS_PER_ELEM) + bit;
    if (bit >= bl->state.max_bits) {
        /*
        ** Last element has extra bits set!
        */
        bl->state.last_elem = bl->state.max_elems;
        return (BITLIST_INVALID);
    }
    return bit + bl->state.base_bit;
}

int
bitlist_find_next(bitlist_t *bl)
{
    return bitlist_find_bit(bl, FALSE);
}

int
bitlist_find_first(bitlist_t *bl)
{
    return bitlist_find_bit(bl, TRUE);
}

int
bitlist_find_after(bitlist_t *bl, int bit)
{
    bit -= bl->state.base_bit;
    if (!bitlist_validbit(bl, bit))
        return (BITLIST_INVALID);
    bl->state.last_elem = BIT_TO_ELEM(bit);
    bl->state.last_bit = BIT_TO_BIT(bit);
    return bitlist_find_bit(bl, FALSE);
}

void
bitlist_copy(bitlist_t *from, bitlist_t *to)
{
    memcpy(to, from, sizeof(bitlist_state_t) +
		     from->state.max_elems * sizeof(bitlist_elem_t));
}


void
bitlist_copy_from_position(bitlist_t *from, bitlist_t *to, int bit, int bits)
{
    if (!bitlist_validrange(from, bit, bits))
	return;
        
    if (!bitlist_validbit(to, bits-1))
        return;

    /*
     * Only copy if the starting bit is an exact multiple of 32  
     */
    if (((bit-(from->state.base_bit))%BITS_PER_ELEM) != 0)
        return;
    
    /*
     * Only copy if the number of bits to copy is an exact multiple of 32  
     */
    if ((bits%BITS_PER_ELEM) != 0)
        return;

    memcpy(&to->bits[0], &from->bits[BIT_TO_ELEM(bit-(from->state.base_bit))], 
           BITS_TO_ELEMS(bits) * sizeof(bitlist_elem_t));
}

void
bitlist_copy_in_position(bitlist_t *from, bitlist_t *to, int bit, int bits)
{
    if (!bitlist_validbit(from, bits-1))
	return;
    
    if (!bitlist_validrange(to, bit, bits))
        return;
    
    /*
     * Only copy if the starting bit is an exact multiple of 32  
     */
    if (((bit-(from->state.base_bit))%BITS_PER_ELEM) != 0)
        return;

    /*
     * Only copy if the number of bits to copy is an exact multiple of 32  
     */
    if ((bits%BITS_PER_ELEM) != 0)
        return;

    memcpy(&to->bits[BIT_TO_ELEM(bit-(to->state.base_bit))], &from->bits[0], 
           BITS_TO_ELEMS(bits) * sizeof(bitlist_elem_t));
}

void
bitlist_or(bitlist_t *a, bitlist_t *b, bitlist_t *c)
{
    bitlist_elem_t  *ap, *bp, *cp;
    int i;

    if (c != a && c != b)
        c->state = a->state;

    ap = &a->bits[0];
    bp = &b->bits[0];
    cp = &c->bits[0];

    for (i = a->state.max_elems; i > 0; --i)
        *cp++ = *ap++ | *bp++;
}

void
bitlist_and(bitlist_t *a, bitlist_t *b, bitlist_t *c)
{
    bitlist_elem_t  *ap, *bp, *cp;
    int i;

    if (c != a && c != b)
        c->state = a->state;

    ap = &a->bits[0];
    bp = &b->bits[0];
    cp = &c->bits[0];

    for (i = a->state.max_elems; i > 0; --i)
        *cp++ = *ap++ & *bp++;
}

void
bitlist_not(bitlist_t *a, bitlist_t *b)
{
    bitlist_elem_t  *ap, *bp;
    int i;

    if (a != b)
        b->state = a->state;

    ap = &a->bits[0];
    bp = &b->bits[0];

    for (i = a->state.max_elems; i > 0; --i)
        *bp++ = ~*ap++;
}

void
bitlist_and_not(bitlist_t *a, bitlist_t *b, bitlist_t *c)
{
    bitlist_elem_t  *ap, *bp, *cp;
    int i;

    if (c != a && c != b)
        c->state = a->state;

    ap = &a->bits[0];
    bp = &b->bits[0];
    cp = &c->bits[0];

    for (i = a->state.max_elems; i > 0; --i)
        *cp++ = *ap++ & ~*bp++;
}

void
bitlist_xor (bitlist_t *a, bitlist_t *b, bitlist_t *c)
{
    bitlist_elem_t  *ap, *bp, *cp;
    int i;

    if (c != a && c != b)
        c->state = a->state;

    ap = &a->bits[0];
    bp = &b->bits[0];
    cp = &c->bits[0];

    for (i = a->state.max_elems; i > 0; --i)
        *cp++ = *ap++ ^ *bp++;
}

boolean
bitlist_equal(bitlist_t *a, bitlist_t *b)
{
    bitlist_elem_t  *ap, *bp;
    int i;

    ap = &a->bits[0];
    bp = &b->bits[0];

    for (i = a->state.max_elems; i > 0; --i) {
        if (*ap++ != *bp++)
            return FALSE;
    }
    return TRUE;
}

int
bitlist_count (bitlist_t *bl)
{
    bitlist_elem_t bits;
    int            elem, bit, count;

    count = 0;
    for (elem = 0; elem < bl->state.max_elems; elem++) {
        bits = bl->bits[elem];
        for (bit = 0; bit < BITS_PER_ELEM; ++bit) {
            count += (bits & (1 << bit)) != 0;
        }
    }
    return (count);
}

int
bitlist_size (bitlist_t *bl)
{
    /*
     * The size of a bitlist is the number of bits that can be set
     */
    return bl->state.max_bits;
}

bitlist_t *
bitlist_alloc(int bits, int base)
{
    bitlist_t *bl;

    bl = malloc(sizeof(bitlist_state_t) +
                BITS_TO_ELEMS(bits) * sizeof(bitlist_elem_t));
    if (bl == NULL)
        return NULL;

    bitlist_clearall(bl, bits, base);
    return bl;
}

bitlist_t *
bitlist_alloc_vect(int bits, int base, int cells)
{
    bitlist_t *bl;
    int i;

    bl = malloc(cells * (sizeof(bitlist_state_t) +
                BITS_TO_ELEMS(bits) * sizeof(bitlist_elem_t)));
    if (bl == NULL)
        return NULL;

    for (i = 0; i < cells; i++)
        bitlist_clearall(bl+i, bits, base);
    return bl;
}

bitlist_t *
bitlist_clone(bitlist_t *bl, boolean copy_bitlist)
{
    bitlist_t *new_bl;

    new_bl = bitlist_alloc(bl->state.max_bits, bl->state.base_bit);
    if (new_bl == NULL)
        return NULL;

    if (copy_bitlist)
        bitlist_copy(bl, new_bl);

    return new_bl;
}

void
bitlist_free_vect(bitlist_t *bl, int cells)
{
    int i;

    for (i = 0; i < cells; i++)
        memset(&(bl+i)->state, 0, sizeof(bitlist_state_t));
    free(bl);
}

void
bitlist_free(bitlist_t *bl)
{
    memset(&bl->state, 0, sizeof(bitlist_state_t));
    free(bl);
}

void 
bitlist_dump (bitlist_t *bl)
{
    int         bit, previous = 0;
    int         begin, end;
    
    bit = bitlist_find_first(bl);
    previous = bit - 2;
    begin = bit;
    end = bit - 1;
    while (bit != BITLIST_INVALID) {
        if (bit != (previous+1)) {
            if (begin < end) {
                printf("0x%x-0x%x,", begin, end);
            } else {
                if (begin == end) {
                    printf("0x%x,", begin);
                }
            }
            begin = bit;
            end = bit;
        } else {
            end = bit;
        }
        
        previous = bit;
        bit = bitlist_find_after(bl, bit);
    }
    if (begin < end) {
        printf("0x%x-0x%x", begin, end);
    } else {
        if (begin == end) {
            printf("0x%x", begin);
        }
    }
}

boolean
bitlist_from_string (bitlist_t* bl, char* list, char** errmsg)
{
    int number = 0, begin = 0, charno = 1;
    int i, min, max;
    enum { none, 
           non_numeric, 
           out_of_bounds, 
           inv_range, 
           exp_comma, 
           unexpected_eol 
    } error;
    int errmsglen = 128;
    
    min = bl->state.base_bit;
    max = min + bl->state.max_bits - 1;
    error = none;

    while (*list) {
        if (*list < '0' || *list > '9') {
            error = non_numeric;
            break;
        }
        number = 0;
        while ((*list >= '0') && (*list <= '9')) {
            number = number * 10 + (*list) - '0';
            list++;
            charno++;
        }
        if ((number < min) || (number > max)) {
            error = out_of_bounds;
            break;
        }

        if (*list == '-') {
            begin = number;
            number = 0;
            list++;
            charno++;

            if (*list == 0) {
                error = unexpected_eol;
                break;
            } else if (*list < '0' || *list > '9') {
                error = non_numeric;
                break;
            }
            while ((*list >= '0') && (*list <= '9')) {
                number = number * 10 + (*list) - '0';
                list++;
                charno++;
            }
            if ((number < min) || (number > max)) {
                error = out_of_bounds;
                break;
            } else if (begin >= number) {
                error = inv_range;
                break;
            }
            for (i = begin; i <= number; i++)
                bitlist_set(bl, i);
        } else {
            bitlist_set(bl, number);
        }
        if (*list != 0) {
            if (*list == ',') {
                list++; 
                charno++;
                if (*list == 0) {
                    error = unexpected_eol;
                    break;
                }
            } else {
                error = exp_comma;
                break;
            }
        }
    }

    if (error == none) {
        *errmsg = NULL;
        return TRUE;
    } else {
        *errmsg = (char*) malloc(errmsglen);
        if (*errmsg == NULL) {
            return FALSE;
        }
    }

    switch (error) {
    case non_numeric:
        snprintf(*errmsg, errmsglen, "Character #%d \'%c\' is non-numeric", 
                 charno, *list);
        break;
    case out_of_bounds:
        snprintf(*errmsg, errmsglen, "Character #%d delimits a number" 
                 " which is out of the range (%d..%d)", charno, min, max);
        break;
    case inv_range:
        snprintf(*errmsg, errmsglen, "Character #%d delimits ending number(%d) "
                 "of the range, which is not greater than the starting "
                 "number(%d)", charno, number, begin);
        break;
    case exp_comma:
        snprintf(*errmsg, errmsglen, "Comma expected at character #%d", charno);
        break;
    case unexpected_eol:
        snprintf(*errmsg, errmsglen, "Unexpected end of list");
        break;
    default:
        snprintf(*errmsg, errmsglen, "Invalid number list");
        break;
    }

    return (FALSE);
}

#define MAX_DIGITS_PER_BIT  14

int
bitlist_to_string (bitlist_t* bl, char* buf, int size, boolean begin)
{
    int len = 0, tlen = 0;
    int curr, next;
    char* comma = "";
    char temp[MAX_DIGITS_PER_BIT+2]; /* should hold ,num\0 or -num\0 */

    buf[0] = 0;
    next = bitlist_find_bit(bl, begin);
    while (next != BITLIST_INVALID) {
        curr = next;
        tlen = sprintf(temp, "%s%d", comma, curr);
        if (len + tlen >= size) {    /* no space, just reset our position */
            curr = next - bl->state.base_bit - 1;
            bl->state.last_bit  = BIT_TO_BIT(curr);
            bl->state.last_elem = BIT_TO_ELEM(curr);
            break;
        }
        strcpy(buf, temp);
        buf += tlen;
        len += tlen;
        comma = ",";
        next = bitlist_find_bit(bl, FALSE);

        if (curr+1 == next) {
            int first = curr;
            do { /* skip to the end of this range */
                curr = next;
                next = bitlist_find_bit(bl, FALSE);
            } while (curr+1 == next);
            tlen = sprintf(temp, "-%d", curr);
            if (len + tlen < size) { /* okay, print the trailing part */
                strcpy(buf, temp);
                buf += tlen;
                len += tlen;
            } else {                 /* no space, just reset our position */
                next = first+1;
                curr = first - bl->state.base_bit;
                bl->state.last_bit  = BIT_TO_BIT(curr);
                bl->state.last_elem = BIT_TO_ELEM(curr);
                break;
            }
        }
    }
    /* return BITLIST_INVALID if we could not print anything */
    return ((len != 0) ? next : BITLIST_INVALID);
}

boolean
bitlist_nv_write (bitlist_t* bl, char* prefix, char* suffix, int maxlen)
{
    boolean begin = TRUE;
    int i, len;
    char *buf, local[80];

    len = maxlen - (strlen(prefix) + 1 + strlen(suffix));
    if (len <= 80) {
        buf = local;
    } else {
        buf = malloc(len);
        if (!buf) {
            return FALSE;
        }
    }
    do {
        i = bitlist_to_string(bl, buf, len, begin);
        begin = FALSE;
        if (buf[0] != 0) {
            nv_write(TRUE, "%s %s%s", prefix, buf, suffix);
        }
    } while (i != BITLIST_INVALID);

    if (buf != local) 
        free(buf);
    return TRUE;
}

boolean
bitlist_anybit_set (bitlist_t *bl)
{
    int              elem;

    for (elem = 0; elem < bl->state.max_elems; elem++) {
        if (bl->bits[elem])
            return TRUE;
    }
    return FALSE;
}
