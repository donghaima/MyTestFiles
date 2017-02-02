/*------------------------------------------------------------------
 * Lightweight allocation bitmap definitions.
 *
 * April 1998, Tim Iverson
 *
 * Copyright (c) 1998 by cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 * This file just lumps all TinyROM's lbm_*() functions together
 * into one module.  There are nicer ways of sharing code between
 * IOS and ROM, but they take more time to implement ...
 */


#define _IOSINCLUDE	1	/* we're compiling for IOS not TinyROM */
#include "../tinyrom/src/lbm_assert.c"
#include "../tinyrom/src/lbm_freeL.c"
#include "../tinyrom/src/lbm_freeR.c"
#include "../tinyrom/src/lbm_allocN.c"
#include "../tinyrom/src/lbm_alloc1.c"
#include "../tinyrom/src/lbm_test1.c"
#include "../tinyrom/src/lbm_testN.c"
#include "../tinyrom/src/lbm_countN.c"
#include "../tinyrom/src/lbm_free1.c"
#include "../tinyrom/src/lbm_freeN.c"
#include "../tinyrom/src/lbm_mark1.c"
#include "../tinyrom/src/lbm_markN.c"
#include "../tinyrom/src/lbm_init.c"
