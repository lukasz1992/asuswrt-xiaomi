
#ifndef EIP93_CLIB_H
#define EIP93_CLIB_H

/* guaranteed APIs:

    memcpy
    memmove
    memset
    memcmp
    offsetof

*/


/* Note: This is a template. Copy and customize according to your needs! */
#if defined(linux) && defined(MODULE)

#include <linux/string.h>     // memmove and memcpy
#include <linux/stddef.h>     // offsetof

#else

#include <string.h>     // memmove
#include <memory.h>     // memcpy, etc.
#include <stddef.h>     // offsetof

#endif

#endif 

