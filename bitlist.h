/* 
 *------------------------------------------------------------------
 * bitlist.h - dynamic bitlist header file
 *
 * November 1998, Steve Larson
 * 
 * Copyright (c) 1998-2007 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#ifndef __BITLIST_H__
#define __BITLIST_H__

#include COMP_INC(arch-ios, cdefs.h)

/*
** SEE BOTTOM OF FILE FOR AN EXAMPLE OF HOW TO USE THESE BITLISTS
*/

/*
** Operations used internally on bitlists
*/
#define BITS_PER_ELEM           32
#define BITS_TO_ELEMS(bits)     (((bits)+BITS_PER_ELEM-1)/BITS_PER_ELEM)
#define BIT_TO_ELEM(bit)        ((bit)/BITS_PER_ELEM)
#define BIT_TO_BIT(bit)         ((bit)&(BITS_PER_ELEM-1))
#define BIT_TO_MASK(bit)        (1<<BIT_TO_BIT(bit))

/*
 * Useful define for returning 'error' when no bit is found etc...
 */
#define BITLIST_INVALID -1

/*
** Pay no attention to the next 5 lines which simply define
** the type for bitlist_elem_t (i.e. uint32)
*/
#define __B(size)               __CONCAT(uint, size)
typedef __B(BITS_PER_ELEM)      bitlist_elem_t;
#undef  __B


/*
** Header for the bitlist to maintain context for "find"
*/
typedef struct {
    uint32  max_elems;
    uint32  max_bits;
    uint32  base_bit;
    uint32  last_elem;
    uint32  last_bit;
} bitlist_state_t;


typedef struct {
    bitlist_state_t  state;
    bitlist_elem_t   bits[0];
} bitlist_t;


boolean bitlist_validbit(bitlist_t *bl, int bit);
boolean bitlist_validrange(bitlist_t *bl, int bit, int bits);
boolean bitlist_test(bitlist_t *bl, int bit);
void    bitlist_set(bitlist_t *bl, int bit);
void    bitlist_clear(bitlist_t *bl, int bit);
void    bitlist_setall(bitlist_t *bl, int bits, int base);
void    bitlist_clearall(bitlist_t *bl, int bits, int base);
void    bitlist_set_contiguous(bitlist_t *bl, int bit, int bits);
void    bitlist_clear_contiguous(bitlist_t *bl, int bit, int bits);
void    bitlist_clearall2(bitlist_t *bl);
int     bitlist_find_bit(bitlist_t *bl, boolean first);
int     bitlist_find_next(bitlist_t *bl);
int     bitlist_find_first(bitlist_t *bl);
int     bitlist_find_after(bitlist_t *bl, int bit);
void    bitlist_copy(bitlist_t *from, bitlist_t *to);
void    bitlist_copy_in_position(bitlist_t *from, bitlist_t *to, int bit, 
                                 int bits);
void    bitlist_copy_from_position(bitlist_t *from, bitlist_t *to, int bit, 
                                   int bits);
void    bitlist_or(bitlist_t *a, bitlist_t *b, bitlist_t *c);
void    bitlist_and(bitlist_t *a, bitlist_t *b, bitlist_t *c);
void    bitlist_not(bitlist_t *a, bitlist_t *b);
void    bitlist_and_not(bitlist_t *a, bitlist_t *b, bitlist_t *c);
void    bitlist_xor(bitlist_t *a, bitlist_t *b, bitlist_t *c);
boolean bitlist_equal(bitlist_t *a, bitlist_t *b);
int     bitlist_count(bitlist_t *bl);
int     bitlist_size(bitlist_t *bl);
bitlist_t *bitlist_alloc(int bits, int base);
bitlist_t *bitlist_alloc_vect(int bits, int base, int cells);
bitlist_t *bitlist_clone(bitlist_t *bl, boolean copy_bitlist);
void    bitlist_free(bitlist_t *bl);
void    bitlist_free_vect(bitlist_t *bl, int cells);
void    bitlist_dump(bitlist_t *bl);
boolean bitlist_anybit_set(bitlist_t *b1);

#define DEF_BITLIST(type, size)                                 \
    typedef struct {                                            \
        bitlist_state_t  state;                                 \
        bitlist_elem_t   bits[BITS_TO_ELEMS(size)];             \
    } type;

/*
** Convert a string representation to bitlist. bl must be allocated 
** and initialized by the caller. If the string is invalid return value 
** is FALSE and errmsg will point to a message string that should be 
** freed by the caller.
*/
boolean bitlist_from_string(bitlist_t* bl, char* list, char** errmsg);

/*
** Convert a bitlist to a compact string. Conversion starts from the
** beginning if begin==TRUE, else it is restarted from the last position
** set by the previous call. Return value -1 indicates the end of list,
** if not the buffer is full and more calls to this routine are required.
**
** Example: if bitlist contains 1-3,5,7-15,20 and size = 8, then output
** of first call will be "1-3,5,7" return value 7, second call "8-15,20"
** return value -1.
**
** Caveat: buffer should be big enough to hold the string representation
** of any indiviual bit, otherwise conversion stops abruptly and -1 is 
** returned.
*/
int bitlist_to_string(bitlist_t* bl, char* buf, int size, boolean begin);

/*
** NVGEN commands that contain a number list. prefix and suffix 
** must be non-null. prefix is usually csb->nv_command, so a space
** is added before printing the range. suffix if non-empty should 
** start with a blank. maxlen specifies the maximum length of the 
** command line including prefix and suffix. If the bitlist string
** representation with prefix and suffix exceeds maxlen, then
** this routine will write multiple commands. 
** 
** Example: if bitlist contains 1-3,5,7-15,20, maxlen=15, prefix
** is "some cmd", suffix="",  then following lines are nv_written 
**   some cmd 1-3,5
**   some cmd 7-15
**   some cmd 20
**
** Optimized for maxlen <= 80. Malloc is needed for a temporary buffer 
** if maxlen - strlen(prefix) + strlen(suffix) > 80. Returns FALSE if 
** malloc fails.
**
** Caveat: maxlen - strlen(prefix)+1+strlen(suffix) should be big
** enough to hold string representation of any individual bit.
*/
boolean bitlist_nv_write(bitlist_t* bl, char* prefix, char* suffix, 
                         int maxlen);


/*
** The bitlist routines are not intended to be used directly but
** rather through a set of wrapper functions.
**
** The wrappers will enforce type checking and do some bookkeeping
** so the user does not have to.
**
** The following example will create a set of wrappers for a bitlist
** called xyz_bitlist_t and has 128 bits.
**
** NOTE:
** If XYZ_BASE is set to 0 then the possible bit values are 0 .. 127
** If XYZ_BASE is set to 1 then the possible bit values are 1 .. 128
** If XYZ_BASE is set to n then the possible bit values are n .. (n + 128 - 1)
** 
** #define XYZ_BASE         0
** #define XYZ_MAX_BITS     128
** DEF_BITLIST(xyz_bitlist_t, XYZ_MAX_BITS);
** 
** 
** static inline boolean
** xyz_bitlist_test(xyz_bitlist_t *bl, int bit)
** {
**     return bitlist_test((bitlist_t *) bl, bit);
** }
** 
** static inline void
** xyz_bitlist_set(xyz_bitlist_t *bl, int bit)
** {
**     bitlist_set((bitlist_t *) bl, bit);
** }
** 
** static inline void
** xyz_bitlist_clear(xyz_bitlist_t *bl, int bit)
** {
**     bitlist_clear((bitlist_t *) bl, bit);
** }
** 
** static inline void
** xyz_bitlist_setall(xyz_bitlist_t *bl)
** {
**     bitlist_setall((bitlist_t *) bl, XYZ_MAX_BITS, XYZ_BASE);
** }
** 
** static inline void
** xyz_bitlist_clearall(xyz_bitlist_t *bl)
** {
**     bitlist_clearall((bitlist_t *) bl, XYZ_MAX_BITS, XYZ_BASE);
** }
** 
** static inline int
** xyz_bitlist_size(void)
** {
**     return XYZ_MAX_BITS;
** }
** 
** static inline int
** xyz_bitlist_find_first(xyz_bitlist_t *bl)
** {
**     return bitlist_find_first((bitlist_t *) bl);
** }
** 
** static inline int
** xyz_bitlist_find_next(xyz_bitlist_t *bl)
** {
**     return bitlist_find_next((bitlist_t *) bl);
** }
** 
** static inline int
** xyz_bitlist_find_after(xyz_bitlist_t *bl, int bit)
** {
**     return bitlist_find_after((bitlist_t *) bl, bit);
** }
** 
** static inline void
** xyz_bitlist_copy(xyz_bitlist_t *from, xyz_bitlist_t *to)
** {
**     bitlist_copy((bitlist_t *) from, (bitlist_t *) to);
** }
**
*/
#endif __BITLIST_H__
