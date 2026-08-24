/****************************************************************************
 * Copyright (c) 2024 Hidegi
 *
 * This software is provided ‘as-is’, without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ****************************************************************************/
#include "RB/io/internal.h"
#include <zlib.h>

#if defined(BMT_COMPILER_CLANG) || defined(BMT_COMPILER_GNUC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wformat"
#endif

#define BMT_READ_GENERIC(dst, n, scanner, fail)                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (*length < (n))                                                                                             \
        {                                                                                                              \
            bmt_error_flag = BMT_STATUS_READ_ERROR;                                                                    \
            fail;                                                                                                      \
        }                                                                                                              \
        *memory = scanner((dst), *memory, (n));                                                                        \
        *length -= n;                                                                                                  \
    } while (0)

#define ne2be _bmt_big_endian_to_native_endian
#define be2ne _bmt_big_endian_to_native_endian

static SPbool _bmt_is_little_endian()
{
    SPuint16 t = 0x0001;
    SPchar c[2];
    memcpy(c, &t, sizeof t);
    return c[0];
}

static void* _bmt_swap_bytes(void* s, SPsize length)
{
    for (SPchar *b = s, *e = b + length - 1; b < e; b++, e--)
    {
        SPchar t = *b;
        *b = *e;
        *e = t;
    }
    return s;
}

static void* _bmt_big_endian_to_native_endian(void* s, SPsize len)
{
    return _bmt_is_little_endian() ? _bmt_swap_bytes(s, len) : s;
}

static const void* _bmt_memcpy(void* dst, const void* src, SPsize n)
{
    memcpy(dst, src, n);
    return (const SPchar*)src + n;
}

static const void* _bmt_swapped_memcpy(void* dst, const void* src, SPsize n)
{
    const void* ret = _bmt_memcpy(dst, src, n);
    return ne2be(dst, n), ret;
}

SPbuffer _bmt_compress(const void* memory, SPsize length)
{
    const SPsize CHUNK_SIZE = 4096;
    SPbuffer buffer = BMT_BUFFER_INIT;

    z_stream stream = {
        .zalloc = Z_NULL, .zfree = Z_NULL, .opaque = Z_NULL, .next_in = (void*)memory, .avail_in = length};

    if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15, 8, Z_DEFAULT_STRATEGY) != Z_OK)
    {
        bmt_error_flag = BMT_STATUS_WRITE_ERROR;
        return BMT_BUFFER_INIT;
    }

    if (stream.avail_in != length)
    {
        bmt_error_flag = BMT_STATUS_WRITE_ERROR;
        deflateEnd(&stream);
        return buffer;
    }

    do
    {
        if (!spBufferReserve(&buffer, buffer.length + CHUNK_SIZE))
        {
            bmt_error_flag = BMT_STATUS_NO_MEMORY;
            spBufferFree(&buffer);
            return BMT_BUFFER_INIT;
        }

        stream.next_out = buffer.data + buffer.length;
        stream.avail_out = CHUNK_SIZE;

        if (deflate(&stream, Z_FINISH) == Z_STREAM_ERROR)
        {
            bmt_error_flag = BMT_STATUS_WRITE_ERROR;
            spBufferFree(&buffer);
            return BMT_BUFFER_INIT;
        }

        buffer.length += CHUNK_SIZE - stream.avail_out;
    } while (!stream.avail_out);

    deflateEnd(&stream);
    return buffer;
}

SPbuffer _bmt_decompress(const void* memory, SPsize length)
{
    const SPsize CHUNK_SIZE = 4096;
    SPbuffer buffer = BMT_BUFFER_INIT;

    z_stream stream = {
        .zalloc = Z_NULL, .zfree = Z_NULL, .opaque = Z_NULL, .next_in = (void*)memory, .avail_in = length};

    if (inflateInit2(&stream, 47) != Z_OK)
    {
        bmt_error_flag = BMT_STATUS_READ_ERROR;
        return BMT_BUFFER_INIT;
    }

    SPint zlib_ret;
    do
    {
        if (!spBufferReserve(&buffer, buffer.length + CHUNK_SIZE))
        {
            bmt_error_flag = BMT_STATUS_NO_MEMORY;
            spBufferFree(&buffer);
            inflateEnd(&stream);
            return BMT_BUFFER_INIT;
        }

        // Comprehensive zip bomb protection
        if (buffer.length > BMT_MAX_DECOMPRESSED_SIZE)
        {
            bmt_error_flag = BMT_STATUS_READ_ERROR;
            spBufferFree(&buffer);
            inflateEnd(&stream);
            return BMT_BUFFER_INIT;
        }

        // Additional protection: check decompression ratio
        // Reject if decompressed size > 100x compressed size
        if (length > 0 && buffer.length > length * 100)
        {
            bmt_error_flag = BMT_STATUS_READ_ERROR;
            spBufferFree(&buffer);
            inflateEnd(&stream);
            return BMT_BUFFER_INIT;
        }

        stream.avail_out = CHUNK_SIZE;
        stream.next_out = (SPubyte*)buffer.data + buffer.length;

        switch ((zlib_ret = inflate(&stream, Z_NO_FLUSH)))
        {
            case Z_MEM_ERROR:
            case Z_DATA_ERROR:
            case Z_NEED_DICT:
            {
                bmt_error_flag = BMT_STATUS_READ_ERROR;
                spBufferFree(&buffer);
                inflateEnd(&stream);
                return BMT_BUFFER_INIT;
            }
            default:
                buffer.length += CHUNK_SIZE - stream.avail_out;
        }
    } while (!stream.avail_out);

    if (zlib_ret != Z_STREAM_END)
    {
        bmt_error_flag = BMT_STATUS_READ_ERROR;
        spBufferFree(&buffer);
        inflateEnd(&stream);
        return BMT_BUFFER_INIT;
    }

    inflateEnd(&stream);
    return buffer;
}

static void* _bmt_alloc_chunk(SPsize size)
{
    // Prevent unbounded allocations
    if (size > BMT_MAX_BUFFER_SIZE)
    {
        bmt_error_flag = BMT_STATUS_BAD_VALUE;
        return NULL;
    }

    SPbyte* ptr = NULL;
    BMT_CHECKED_CALLOC(ptr, size, sizeof(SPbyte), return NULL);
    return ptr;
}

static void _bmt_write_bytes(SPbuffer* buffer, const SPubyte* src, SPsize length, SPbool toNativeEndian, int level)
{
    if (!buffer || !src)
    {
        bmt_error_flag = BMT_STATUS_BAD_VALUE;
        return;
    }

    SPubyte* chunk = _bmt_alloc_chunk(length);
    if (!chunk)
    {
        bmt_error_flag = BMT_STATUS_NO_MEMORY;
        return;
    }

    if (toNativeEndian)
        _bmt_swapped_memcpy(chunk, src, length);
    else
        _bmt_memcpy(chunk, src, length);

    if (!spBufferAppend(buffer, chunk, length))
    {
        free(chunk);
        bmt_error_flag = BMT_STATUS_WRITE_ERROR;
        return;
    }

    free(chunk);
}

static void _bmt_encode(const BMT_node node, SPbuffer* buffer, int level)
{
    if (node)
    {
        switch (node->tag)
        {
            case BMT_TAG_BYTE:
                _bmt_write_bytes(buffer, (const SPubyte*)&node->payload.tag_byte, sizeof(SPbyte), BMT_TRUE, level);
                break;
            case BMT_TAG_SHORT:
                _bmt_write_bytes(buffer, (const SPubyte*)&node->payload.tag_short, sizeof(SPshort), BMT_TRUE, level);
                break;
            case BMT_TAG_INT:
                _bmt_write_bytes(buffer, (const SPubyte*)&node->payload.tag_int, sizeof(SPint), BMT_TRUE, level);
                break;
            case BMT_TAG_LONG:
                _bmt_write_bytes(buffer, (const SPubyte*)&node->payload.tag_long, sizeof(SPlong), BMT_TRUE, level);
                break;
            case BMT_TAG_FLOAT:
                _bmt_write_bytes(buffer, (const SPubyte*)&node->payload.tag_float, sizeof(SPfloat), BMT_TRUE, level);
                break;
            case BMT_TAG_DOUBLE:
                _bmt_write_bytes(buffer, (const SPubyte*)&node->payload.tag_double, sizeof(SPdouble), BMT_TRUE, level);
                break;
            case BMT_TAG_STRING:
            {
                _bmt_write_bytes(buffer, (const SPubyte*)&node->length, sizeof(SPsize), BMT_TRUE, level);
                _bmt_write_bytes(buffer, (const SPubyte*)node->payload.tag_string, node->length, BMT_FALSE, level);
                break;
            }

            case BMT_TAG_ROOT:
            {
                _bmt_encode_tree(node->payload.tag_object, buffer, level + 1);
                break;
            }

            default:
            {
                SPsize length = _bmt_length_of_list(node->payload.tag_object);
                _bmt_write_bytes(buffer, (const SPubyte*)&node->length, sizeof(SPsize), BMT_TRUE, level);
                _bmt_encode_list(node->payload.tag_object, buffer, level);
                break;
            }
        }
    }
}

void _bmt_encode_tree(const BMT_node tree, SPbuffer* buffer, int level)
{
    if (!tree)
    {
        BMT_tag null = BMT_TAG_NULL;
        _bmt_write_bytes(buffer, (const SPubyte*)&null, sizeof(SPbyte), BMT_FALSE, level);
        return;
    }

    // encode the tag
    SPuint8 tag = tree->tag | ((tree->red & 1) << 6);
    _bmt_write_bytes(buffer, (const SPubyte*)&tag, sizeof(SPbyte), BMT_FALSE, level);

    // encode the name
    _bmt_write_bytes(buffer, &tree->nameLength, sizeof(SPubyte), BMT_FALSE, level);
    _bmt_write_bytes(buffer, (const SPubyte*)tree->name, tree->nameLength, BMT_FALSE, level);

    // encode generic
    _bmt_encode(tree, buffer, level);

    _bmt_encode_tree(tree->major, buffer, level + 1);
    _bmt_encode_tree(tree->minor, buffer, level + 1);
}

void _bmt_encode_list(const BMT_node list, SPbuffer* buffer, int level)
{
    if (list)
    {
        BMT_node cursor = NULL;
        SPsize i = 0;

        for (cursor = list; cursor != NULL; cursor = cursor->major)
        {
            if (cursor->tag & BMT_TAG_LIST)
            {
                // encode the tag, if it is a multi-list
                SPuint8 tag = list->tag | ((list->red & 1) << 6);
                _bmt_write_bytes(buffer, (const SPubyte*)&tag, sizeof(SPbyte), BMT_FALSE, level);
            }
            _bmt_encode(cursor, buffer, level);
        }
    }
}

SPbuffer bmt_EncodeTree(const BMT_node tree)
{
    SPbuffer buffer = BMT_BUFFER_INIT;

    if (!bmt_IsTree(tree))
    {
        bmt_error_flag = BMT_STATUS_NOT_A_TREE;
        return buffer;
    }

    _bmt_update(tree);
    _bmt_encode_tree(tree, &buffer, 0);
    SPbuffer compressed = _bmt_compress(buffer.data, buffer.length);
    spBufferFree(&buffer);
    return compressed;
}

static SPbool _bmt_decode(BMT_node node, const SPubyte** memory, SPsize* length)
{
    if (node)
    {
        switch (node->tag)
        {
            case BMT_TAG_BYTE:
                BMT_READ_GENERIC(&node->payload.tag_byte, sizeof(SPbyte), _bmt_swapped_memcpy, return BMT_FALSE);
                node->length = sizeof(SPbyte);
                break;
            case BMT_TAG_SHORT:
                BMT_READ_GENERIC(&node->payload.tag_short, sizeof(SPshort), _bmt_swapped_memcpy, return BMT_FALSE);
                node->length = sizeof(SPshort);
                break;
            case BMT_TAG_INT:
                BMT_READ_GENERIC(&node->payload.tag_int, sizeof(SPint), _bmt_swapped_memcpy, return BMT_FALSE);
                node->length = sizeof(SPint);
                break;
            case BMT_TAG_LONG:
                BMT_READ_GENERIC(&node->payload.tag_long, sizeof(SPlong), _bmt_swapped_memcpy, return BMT_FALSE);
                node->length = sizeof(SPlong);
                break;
            case BMT_TAG_FLOAT:
                BMT_READ_GENERIC(&node->payload.tag_float, sizeof(SPfloat), _bmt_swapped_memcpy, return BMT_FALSE);
                node->length = sizeof(SPfloat);
                break;
            case BMT_TAG_DOUBLE:
                BMT_READ_GENERIC(&node->payload.tag_double, sizeof(SPdouble), _bmt_swapped_memcpy, return BMT_FALSE);
                node->length = sizeof(SPdouble);
                break;
            case BMT_TAG_STRING:
            {
                // Read as signed integer first to detect negative values
                SPlong stringLength;
                BMT_READ_GENERIC(&stringLength, sizeof(SPlong), _bmt_swapped_memcpy, return BMT_FALSE);
                // Strict bounds check: must be non-negative and leave room for +1 without overflow
                if (stringLength < 0 || stringLength >= BMT_MAX_STRING_LENGTH)
                {
                    bmt_error_flag = BMT_STATUS_READ_ERROR;
                    return BMT_FALSE;
                }
                // Safe to assign to unsigned now
                node->length = (SPsize)stringLength;
                BMT_CHECKED_CALLOC(node->payload.tag_string, node->length + 1, sizeof(SPbyte), return BMT_FALSE);
                BMT_READ_GENERIC(node->payload.tag_string, node->length, _bmt_memcpy, return BMT_FALSE);
                node->payload.tag_string[node->length] = 0;
                break;
            }
            case BMT_TAG_ROOT:
            {
                node->payload.tag_object = _bmt_decode_tree(memory, length);
                break;
            }

            default:
            {
                _bmt_decode_list(node, memory, length);
                break;
            }
        }
    }
    return BMT_TRUE;
}

BMT_node _bmt_decode_tree(const SPubyte** memory, SPsize* length)
{
    SPubyte tag;
    BMT_READ_GENERIC(&tag, sizeof(SPubyte), _bmt_memcpy, return NULL);

    SPbool redness = (tag >> 6) & 1;
    tag &= ~0x40;

    if (tag == BMT_TAG_NULL)
        return NULL;

    BMT_node tree;
    BMT_CHECKED_CALLOC(tree, 1, sizeof(struct _BMT_node), return NULL);

    BMT_READ_GENERIC(&tree->nameLength, sizeof(SPubyte), _bmt_memcpy, {
        free(tree);
        return NULL;
    });
    BMT_CHECKED_CALLOC(tree->name, tree->nameLength + 1, sizeof(SPchar), {
        free(tree);
        return NULL;
    });
    BMT_READ_GENERIC(tree->name, tree->nameLength, _bmt_memcpy, {
        free(tree->name);
        free(tree);
        return NULL;
    });
    tree->name[tree->nameLength] = 0;

    tree->red = redness;
    tree->tag = tag;

    if (!_bmt_decode(tree, memory, length))
    {
        free(tree->name);
        free(tree);
        return NULL;
    }

    tree->major = _bmt_decode_tree(memory, length);
    tree->minor = _bmt_decode_tree(memory, length);

    if (tree->major)
        tree->major->parent = tree;
    if (tree->minor)
        tree->minor->parent = tree;

    return tree;
}

SPbool _bmt_decode_list(BMT_node node, const SPubyte** memory, SPsize* length)
{
    if (!node)
        return BMT_FALSE;

    BMT_READ_GENERIC(&node->length, sizeof(SPlong), _bmt_swapped_memcpy, return BMT_FALSE);
    // Bounds check to prevent memory exhaustion
    if (node->length < 0 || node->length > BMT_MAX_LIST_LENGTH)
    {
        bmt_error_flag = BMT_STATUS_READ_ERROR;
        return BMT_FALSE;
    }
    if (node->length)
    {
        BMT_node array = bmt_AllocList();
        BMT_node cursor = array;

        for (SPsize i = 0; i < node->length; i++)
        {
            SPubyte tag;
            SPbool redness = BMT_FALSE;
            if (node->tag == BMT_TAG_LIST)
            {
                BMT_READ_GENERIC(&tag, sizeof(SPbyte), _bmt_memcpy, return BMT_FALSE);
                redness = (tag >> 6) & 1;
                if (!redness)
                {
                    bmt_Delete(&array);
                    return BMT_FALSE;
                }
                tag &= ~0x40;
            }
            else
            {
                tag = node->tag & ~BMT_TAG_LIST;
                redness = BMT_TRUE;
            }

            cursor->tag = tag;
            cursor->red = redness;

            if (!_bmt_decode(cursor, memory, length))
            {
                bmt_Delete(&array);
                return BMT_FALSE;
            }

            cursor->major = bmt_AllocList();
            BMT_node minor = cursor;
            cursor = cursor->major;
            cursor->minor = minor;
        }

        if (cursor->minor)
            cursor->minor->major = NULL;

        free(cursor);
        node->payload.tag_object = array;
    }
    return BMT_TRUE;
}

BMT_node bmt_DecodeTree(SPbuffer buffer)
{
    SPbuffer decompressed = _bmt_decompress(buffer.data, buffer.length);
    const SPubyte* memory = decompressed.data;
    BMT_node ret = _bmt_decode_tree((const SPubyte**)&memory, &decompressed.length);
    spBufferFree(&decompressed);
    return ret;
}
#if defined(BMT_COMPILER_CLANG) || defined(BMT_COMPILER_GNUC)
#pragma GCC diagnostic pop
#endif
