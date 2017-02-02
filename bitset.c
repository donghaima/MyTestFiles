/*
 * bitset.c -- bitset sample implementation
 * Copyright (C) 2004-2006, Davide Angelocola <davide.angelocola@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

/* 
 * This is defined conditionally to allow the programmer to provide a 
 * "custom" size.
 */
#ifndef BS_SIZE
# define BS_SIZE    1024
#endif

/* Number of bits per word of bitset structure. */
#define BS_NBITS       (sizeof(long) * 8)

/* bitset structure. */
typedef struct _bitset bitset;

struct _bitset {
    long bits[BS_SIZE / BS_NBITS + 1];
};

/* 
 * bitset manipulation macros 
 *   `n' is the n-th bit
 *   `p' is a pointer to a `bitset' structure 
 */
#define BS_SET(n,p)    ((p)->bits[(n) / BS_NBITS] |= (1 << ((n) % BS_NBITS)))
#define BS_CLR(n,p)    ((p)->bits[(n) / BS_NBITS] &= ~(1 << ((n) % BS_NBITS)))
#define BS_ISSET(n,p)  ((p)->bits[(n) / BS_NBITS] & (1 << ((n) % BS_NBITS)))
#define BS_ZERO(p)     memset((p), '\0', sizeof(bitset))


/* Test program. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

int
main(void)
{
  register int i;
  bitset x;
  
  /* Clearing bits. */
  BS_ZERO(&x);
   
  /* Testing BS_ZERO. */
  for (i = 0; i < BS_SIZE; i++) 
    if (BS_ISSET(i, &x)) {
      printf("test failed (BS_ZERO).\n");
      return EXIT_FAILURE;
    }
 
  /* Testing BS_SET and BS_CLR. */      
  for (i = 0; i < BS_SIZE; i++) {
    /* Setting the i-th bit. */
    BS_SET(i, &x);

    if (!BS_ISSET(i, &x)) {
      printf("test failed (BS_SET).\n");
      return EXIT_FAILURE;
    }
    
    /* Clearing the i-th bit. */
    BS_CLR(i, &x);
   
    if (BS_ISSET(i, &x)) {
      printf("test failed (BS_CLR).\n");
      return EXIT_FAILURE;
    }
  }
 
  /* All tests was successful. */ 
  printf("test ok.\n");
  return EXIT_SUCCESS;
}

