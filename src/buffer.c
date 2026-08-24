/*
 * -----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Lukas Niederbremer <webmaster@flippeh.de> and Clark Gaebel <cg.wowus.cg@gmail.com>
 * wrote this file. As long as you retain this notice you can do whatever you
 * want with this stuff. If we meet some day, and you think this stuff is worth
 * it, you can buy us a beer in return.
 * -----------------------------------------------------------------------------
 */
#include "buffer.h"
#include "bmt.h"
#include "internal.h"

#ifdef BMT_COMPILER_GNUC
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

static int _spLazyInit(BMT_buffer* b)
{
    assert(!b->data && "Buffer already allocated");
    BMT_size capacity = 1024;
    b->data = malloc(capacity);
    b->length = 0;
    b->capacity = capacity;

    return unlikely(!b->data) ? BMT_FALSE : BMT_TRUE;
}

void bmt_BufferFree(BMT_buffer* b)
{
    if (b->data)
    {
        free(b->data);
        b->data = NULL;
        b->length = b->capacity = 0;
    }
}

BMT_bool bmt_BufferReserve(BMT_buffer* b, BMT_size reserved)
{
    assert(b && "Cannot reserve for NULL");

    // Early bounds check before any allocation
    if (reserved > BMT_MAX_BUFFER_SIZE)
    {
        return BMT_FALSE;
    }

    if (unlikely(!b->data) && unlikely(!_spLazyInit(b)))
    {
        return BMT_FALSE;
    }
    if (likely(b->capacity >= reserved))
        return BMT_TRUE;

    BMT_size targetCapacity = b->capacity;
    while (targetCapacity < reserved)
    {
        // Check for overflow before doubling
        if (targetCapacity > BMT_MAX_BUFFER_SIZE / 2)
        {
            // Would overflow, use reserved directly
            targetCapacity = reserved;
            break;
        }
        BMT_size newCapacity = targetCapacity * 2;
        if (newCapacity > BMT_MAX_BUFFER_SIZE)
        {
            newCapacity = reserved;
        }
        targetCapacity = newCapacity;
    }

    unsigned char* tmp = realloc(b->data, targetCapacity);
    if (unlikely(!tmp))
    {
        bmt_BufferFree(b);
        return BMT_FALSE;
    }

    b->data = tmp;
    b->capacity = targetCapacity;
    return BMT_TRUE;
}

BMT_bool bmt_BufferAppend(BMT_buffer* b, const void* data, BMT_size n)
{
    assert(b && "Cannot append to NULL");
    if (unlikely(!b->data) && unlikely(!_spLazyInit(b)))
    {
        return BMT_FALSE;
    }

    // Check for overflow in addition before reserve
    if (n > BMT_MAX_BUFFER_SIZE - b->length)
    {
        return BMT_FALSE;
    }

    if (unlikely(!bmt_BufferReserve(b, b->length + n)))
    {
        return BMT_FALSE;
    }

    memcpy(b->data + b->length, data, n);
    b->length += n;
    return BMT_TRUE;
}