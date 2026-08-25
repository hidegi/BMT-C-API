/*
 * -----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Lukas Niederbremer <webmaster@flippeh.de> and Clark Gaebel <cg.wowus.cg@gmail.com>
 * wrote this file. As long as you retain this notice you can do whatever you
 * want with this stuff. If we meet some day, and you think this stuff is worth
 * it, you can buy us a beer in return.
 * -----------------------------------------------------------------------------
 */
#ifndef BMT_BUFFER_H
#define BMT_BUFFER_H
#include "platform.h"
#include "types.h"
#include <stddef.h>

#define BMT_BUFFER_INIT (struct BMT_buffer){NULL, 0, 0}

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct BMT_buffer
    {
            BMT_ubyte* data;
            BMT_size length;
            BMT_size capacity;
    } BMT_buffer;

    BMT_API void bmt_BufferFree(BMT_buffer* b);
    BMT_API BMT_bool bmt_BufferReserve(BMT_buffer* b, BMT_size reserved);
    BMT_API BMT_bool bmt_BufferAppend(BMT_buffer* b, const void* data, BMT_size n);

#ifdef __cplusplus
}
#endif
#endif