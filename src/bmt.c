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
#include "internal.h"
#include <math.h>
#include <stdarg.h>
#include <stdint.h>

#define BMT_HAVE_BST_MAJOR_INCLINED
#if defined(BMT_COMPILER_CLANG) || defined(BMT_COMPILER_GNUC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wformat"
#endif

#define BMT_CHECK_INPUT(_name)                                                                                         \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!tree)                                                                                                     \
        {                                                                                                              \
            bmt_error_flag = BMT_STATUS_BAD_VALUE;                                                                     \
            return;                                                                                                    \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            if ((*tree) && ((*tree)->parent || (*tree)->red))                                                          \
            {                                                                                                          \
                bmt_error_flag = BMT_STATUS_NOT_A_TREE;                                                                \
                return;                                                                                                \
            }                                                                                                          \
        }                                                                                                              \
        if (!_bmt_is_name_valid(_name))                                                                                \
        {                                                                                                              \
            bmt_error_flag = BMT_STATUS_BAD_NAME;                                                                      \
            return;                                                                                                    \
        }                                                                                                              \
    } while (0)

#define BMT_FOR_EACH(node, cursor) for ((cursor) = (node); (cursor) != NULL; (cursor) = (cursor)->major)

#define BMT_COMPARE_GENERIC(p)                                                                                         \
    if (a->payload.p != b->payload.p)                                                                                  \
    return BMT_FALSE

BMT_THREAD_LOCAL BMT_status bmt_error_flag = BMT_STATUS_OK;

BMT_bool bmt_IsTree(const BMT_node node)
{
    BMT_bool ret = BMT_FALSE;
    if (node)
    {
        ret = (node->parent == NULL) && !node->red;
    }
    return ret;
}

BMT_bool bmt_IsList(const BMT_node node)
{
    BMT_bool ret = BMT_FALSE;
    if (node)
    {
        ret = (node->parent == NULL) && node->red;
    }
    return ret;
}

BMT_bool _bmt_is_name_valid(const BMT_char* name)
{
    if (!name)
        return BMT_FALSE;

    BMT_size length = strlen(name);
    return (length > 0) && (length <= ((BMT_ubyte)0xFF));
}

static BMT_tag _bmt_get_object_tag(const BMT_node value)
{
    BMT_tag tag = BMT_TAG_NULL;

    if (bmt_IsList(value) || bmt_IsTree(value))
    {
        if (value->tag != BMT_TAG_NULL)
        {
            if (bmt_IsTree(value))
            {
                tag = BMT_TAG_ROOT;
            }
            else
            {
                if (value->tag & BMT_TAG_LIST)
                    tag = BMT_TAG_LIST;
                else
                    tag = BMT_TAG_LIST | value->tag;
            }
        }
    }

    return tag;
}

static BMT_node
_bmt_alloc_node(BMT_tag tag, const BMT_char* name, BMT_size length, const void* value, BMT_bool copyValue)
{
    BMT_node node = NULL;
    BMT_CHECKED_CALLOC(node, 1, sizeof(struct _BMT_node), return NULL);
    node->tag = tag;

    node->nameLength = strlen(name);
    _bmt_strncpy(&node->name, name, node->nameLength);

    node->length = length;
    node->major = node->minor = NULL;
    node->parent = NULL;
    node->red = BMT_FALSE;

    switch (tag)
    {
        case BMT_TAG_NULL:
            break;
        case BMT_TAG_BYTE:
            node->payload.tag_byte = *(const BMT_byte*)value;
            break;
        case BMT_TAG_SHORT:
            node->payload.tag_short = *(const BMT_short*)value;
            break;
        case BMT_TAG_INT:
            node->payload.tag_int = *(const BMT_int*)value;
            break;
        case BMT_TAG_LONG:
            node->payload.tag_long = *(const BMT_long*)value;
            break;
        case BMT_TAG_FLOAT:
            node->payload.tag_float = *(const BMT_float*)value;
            break;
        case BMT_TAG_DOUBLE:
            node->payload.tag_double = *(const BMT_double*)value;
            break;
        case BMT_TAG_STRING:
        {
            _bmt_strncpy(&node->payload.tag_string, (const BMT_char*)value, node->length);
            break;
        }

        default:
        {
            node->payload.tag_object = copyValue ? bmt_Copy((const BMT_node)value) : (BMT_node)value;
            break;
        }
    }
    return node;
}

static BMT_size _bmt_length_of(BMT_tag tag)
{
    switch (tag)
    {
        case BMT_TAG_BYTE:
            return sizeof(BMT_byte);
        case BMT_TAG_SHORT:
            return sizeof(BMT_short);
        case BMT_TAG_INT:
            return sizeof(BMT_int);
        case BMT_TAG_LONG:
            return sizeof(BMT_long);
        case BMT_TAG_FLOAT:
            return sizeof(BMT_float);
        case BMT_TAG_DOUBLE:
            return sizeof(BMT_double);
        case BMT_TAG_NULL:
            break;
        default:
            break;
    }
    return 0;
}

BMT_node bmt_AllocTree()
{
    BMT_node root = NULL;
    BMT_CHECKED_CALLOC(root, 1, sizeof(struct _BMT_node), return NULL);
    root->length = 0LL;

    root->nameLength = 0;
    root->name = NULL;

    root->tag = BMT_TAG_NULL;
    root->parent = root->major = root->minor = NULL;
    root->red = BMT_FALSE;
    return root;
}

BMT_node bmt_AllocList()
{
    BMT_node list = bmt_AllocTree();
    if (!list)
        return NULL;

    list->red = BMT_TRUE;

    return list;
}

BMT_int _bmt_strcmp(const BMT_char* a, const BMT_char* b)
{
    // Compare the strings lexicographically.
    // Longer strings are majored, shorter strings are minored.
    BMT_size aLen = strlen(a);
    BMT_size bLen = strlen(b);

    if (aLen == bLen)
        return strcmp(a, b);

    if (aLen > bLen)
        return 1;
    else
        return -1;
}

void _bmt_strncpy(BMT_char** dst, const BMT_char* src, BMT_size length)
{
    if (!src)
    {
        bmt_error_flag = BMT_STATUS_BAD_VALUE;
        return;
    }
    if (dst)
    {
        // Validate length before any allocation to prevent overflow
        if (length > BMT_MAX_STRING_LENGTH)
        {
            bmt_error_flag = BMT_STATUS_BAD_VALUE;
            return;
        }

        // Check for overflow in length + 1
        if (length > SIZE_MAX - 1)
        {
            bmt_error_flag = BMT_STATUS_BAD_VALUE;
            return;
        }

        if (!(*dst))
        {
            BMT_CHECKED_CALLOC(*dst, length + 1, sizeof(BMT_char), return);
        }
        else
        {
            BMT_char* ptr = realloc(*dst, length + 1);
            if (!ptr)
            {
                free(*dst);
                *dst = NULL;
                bmt_error_flag = BMT_STATUS_NO_MEMORY;
                return;
            }
            *dst = ptr;
        }

        memcpy(*dst, src, length);
        (*dst)[length] = 0;
    }
}

static void _bmt_update_impl(BMT_node node)
{
    if (node)
    {
        if (node->tag == BMT_TAG_STRING)
        {
            node->length = strlen(node->payload.tag_string);
        }

        if (node->tag == BMT_TAG_ROOT)
        {
            _bmt_update_tree(node->payload.tag_object);
        }

        if (node->tag & BMT_TAG_LIST)
        {
            if (node->payload.tag_object)
                node->tag = _bmt_get_object_tag(node->payload.tag_object);

            node->length = _bmt_length_of_list(node->payload.tag_object);
            _bmt_update_list(node->payload.tag_object);
        }
    }
}

void _bmt_update_tree(BMT_node node)
{
    if (node)
    {
        _bmt_update_impl(node);
        _bmt_update_tree(node->major);
        _bmt_update_tree(node->minor);
    }
}

void _bmt_update_list(BMT_node node)
{
    if (node)
    {
        for (BMT_node cursor = node; cursor != NULL; cursor = cursor->major)
            _bmt_update_impl(cursor);
    }
}

void _bmt_update(BMT_node node)
{
    if (node)
    {
        if (bmt_IsList(node) || bmt_IsTree(node))
        {
            if (bmt_IsTree(node))
                _bmt_update_tree(node);
            else
                _bmt_update_list(node);
        }
    }
}

static BMT_bool _bmt_copy_payload_from_node(const BMT_node src, BMT_node dst)
{
    if (src && dst)
    {
        switch (dst->tag)
        {
            case BMT_TAG_NULL:
                break;
            case BMT_TAG_BYTE:
                dst->payload.tag_byte = src->payload.tag_byte;
                break;
            case BMT_TAG_SHORT:
                dst->payload.tag_short = src->payload.tag_short;
                break;
            case BMT_TAG_INT:
                dst->payload.tag_int = src->payload.tag_int;
                break;
            case BMT_TAG_LONG:
                dst->payload.tag_long = src->payload.tag_long;
                break;
            case BMT_TAG_FLOAT:
                dst->payload.tag_float = src->payload.tag_float;
                break;
            case BMT_TAG_DOUBLE:
                dst->payload.tag_double = src->payload.tag_double;
                break;
            case BMT_TAG_STRING:
            {
                dst->length = src->length;
                _bmt_strncpy(&dst->payload.tag_string, src->payload.tag_string, dst->length);
                break;
            }
            default:
            {
                dst->payload.tag_object = bmt_Copy(src->payload.tag_object);
                if (src->payload.tag_object && !dst->payload.tag_object)
                {
                    // Copy failed, error flag already set
                    return BMT_FALSE;
                }
                break;
            }
        }
    }
    return BMT_TRUE;
}

static BMT_bool
_bmt_copy_payload_from_memory(BMT_node node, BMT_tag tag, BMT_size length, const void* value, BMT_bool copyValue)
{
    if (node)
    {
        switch (tag)
        {
            case BMT_TAG_NULL:
                break;
            case BMT_TAG_BYTE:
                node->payload.tag_byte = *(const BMT_byte*)value;
                node->length = sizeof(BMT_byte);
                break;
            case BMT_TAG_SHORT:
                node->payload.tag_short = *(const BMT_short*)value;
                node->length = sizeof(BMT_short);
                break;
            case BMT_TAG_INT:
                node->payload.tag_int = *(const BMT_int*)value;
                node->length = sizeof(BMT_int);
                break;
            case BMT_TAG_LONG:
                node->payload.tag_long = *(const BMT_long*)value;
                node->length = sizeof(BMT_long);
                break;
            case BMT_TAG_FLOAT:
                node->payload.tag_float = *(const BMT_float*)value;
                node->length = sizeof(BMT_float);
                break;
            case BMT_TAG_DOUBLE:
                node->payload.tag_double = *(const BMT_double*)value;
                node->length = sizeof(BMT_double);
                break;
            case BMT_TAG_STRING:
            {
                // Validate string length before allocation
                if (length >= BMT_MAX_STRING_LENGTH)
                {
                    bmt_error_flag = BMT_STATUS_BAD_VALUE;
                    return BMT_FALSE;
                }
                node->length = length;
                _bmt_strncpy(&node->payload.tag_string, (const BMT_char*)value, length);
                break;
            }

            default:
            {
                node->length = length;
                node->payload.tag_object = copyValue ? bmt_Copy((const BMT_node)value) : (const BMT_node)value;
                if (copyValue && value && !node->payload.tag_object)
                {
                    // Copy failed, error flag already set
                    return BMT_FALSE;
                }
                break;
            }
        }
    }
    return BMT_TRUE;
}

BMT_node _bmt_copy_tree_depth(const BMT_node n, int depth)
{
    BMT_node tree = NULL;
    if (n)
    {
        tree = bmt_AllocTree();
        if (!tree)
        {
            bmt_error_flag = BMT_STATUS_NO_MEMORY;
            return NULL;
        }

        tree->red = n->red;

        tree->nameLength = n->nameLength;
        _bmt_strncpy(&tree->name, n->name, n->nameLength);

        tree->tag = n->tag;
        tree->length = n->length;

        if (!_bmt_copy_payload_from_node(n, tree))
        {
            if (tree->name)
                free(tree->name);
            free(tree);
            return NULL;
        }

        tree->major = _bmt_copy_tree_depth(n->major, depth + 1);
        tree->minor = _bmt_copy_tree_depth(n->minor, depth + 1);

        // Check if recursive copies succeeded
        if ((n->major && !tree->major) || (n->minor && !tree->minor))
        {
            // Recursive copy failed, cleanup
            _bmt_delete_tree_impl(tree);
            return NULL;
        }

        if (tree->major)
            tree->major->parent = tree;

        if (tree->minor)
            tree->minor->parent = tree;
    }
    return tree;
}

static BMT_node _bmt_copy_tree(const BMT_node n)
{
    return _bmt_copy_tree_depth(n, 0);
}

static BMT_node _bmt_copy_list(const BMT_node n)
{
    BMT_node list = NULL;
    if (n)
    {
        if (n->red && !n->parent)
        {
            list = bmt_AllocList();
            if (!list)
            {
                bmt_error_flag = BMT_STATUS_NO_MEMORY;
                return NULL;
            }

            BMT_node dst_cursor = list;
            BMT_node src_cursor = NULL;

            for (src_cursor = n; src_cursor != NULL; src_cursor = src_cursor->major)
            {
                dst_cursor->tag = src_cursor->tag;

                dst_cursor->red = BMT_TRUE;

                if (!_bmt_copy_payload_from_node(src_cursor, dst_cursor))
                {
                    bmt_Delete(&list);
                    return NULL;
                }

                dst_cursor->length = src_cursor->length;
                dst_cursor->major = bmt_AllocList();
                if (!dst_cursor->major)
                {
                    bmt_error_flag = BMT_STATUS_NO_MEMORY;
                    bmt_Delete(&list);
                    return NULL;
                }

                BMT_node minor = dst_cursor;
                dst_cursor = dst_cursor->major;
                dst_cursor->minor = minor;
            }

            // Fix: Check for NULL before dereferencing
            if (dst_cursor && dst_cursor->minor)
            {
                dst_cursor->minor->major = NULL;
            }
            free(dst_cursor);
        }
    }
    return list;
}

BMT_node bmt_Copy(const BMT_node n)
{
    BMT_node ret = NULL;
    if (n)
    {
        if (!(bmt_IsTree(n) || bmt_IsList(n)))
        {
            bmt_error_flag = BMT_STATUS_BAD_VALUE;
            return NULL;
        }
        _bmt_update(n);
        ret = bmt_IsTree(n) ? _bmt_copy_tree(n) : _bmt_copy_list(n);
        if (ret && bmt_error_flag == BMT_STATUS_OK)
        {
            // Success - keep error flag clear
        }
    }
    return ret;
}

BMT_bool _bmt_decimal_almost_equal(BMT_double a, BMT_double b, BMT_size places)
{
    return (fabs(a - b) < pow(10.0, -(BMT_double)places));
}

BMT_bool _bmt_is_equal(const BMT_node a, const BMT_node b)
{
    if (!a && !b)
        return BMT_TRUE;

    if ((!a && b) || (a && !b))
        return BMT_FALSE;

    if (a->tag != b->tag)
        return BMT_FALSE;

    if (a->length != b->length)
        return BMT_FALSE;

    if (a->red != b->red)
        return BMT_FALSE;

    switch (a->tag)
    {
        case BMT_TAG_NULL:
            break;
        case BMT_TAG_BYTE:
            BMT_COMPARE_GENERIC(tag_byte);
            break;
        case BMT_TAG_SHORT:
            BMT_COMPARE_GENERIC(tag_short);
            break;
        case BMT_TAG_INT:
            BMT_COMPARE_GENERIC(tag_int);
            break;
        case BMT_TAG_LONG:
            BMT_COMPARE_GENERIC(tag_long);
            break;
        case BMT_TAG_FLOAT:
        {
            if (!_bmt_decimal_almost_equal(a->payload.tag_float, b->payload.tag_float, 6))
                return BMT_FALSE;
            break;
        }
        case BMT_TAG_DOUBLE:
        {
            if (!_bmt_decimal_almost_equal(a->payload.tag_double, b->payload.tag_double, 15))
                return BMT_FALSE;
            break;
        }

        case BMT_TAG_STRING:
        {
            if (strncmp(a->payload.tag_string, b->payload.tag_string, a->length))
                return BMT_FALSE;
            break;
        }
        case BMT_TAG_ROOT:
        {
            if (!_bmt_is_tree_equal(a->payload.tag_object, b->payload.tag_object))
                return BMT_FALSE;
            break;
        }

        default:
        {
            if (!_bmt_is_list_equal(a->payload.tag_object, b->payload.tag_object))
                return BMT_FALSE;
        }
    }

    return BMT_TRUE;
}

BMT_bool _bmt_is_tree_equal_impl(const BMT_node a, const BMT_node b, int depth)
{
    if (!_bmt_is_equal(a, b))
        return BMT_FALSE;

    if (a && b)
    {
        if (_bmt_strcmp(a->name, b->name) != 0)
        {
            return BMT_FALSE;
        }
    }

    return (a && b) ? (_bmt_is_tree_equal_impl(a->major, b->major, depth + 1)
                       && _bmt_is_tree_equal_impl(a->minor, b->minor, depth + 1))
                    : BMT_TRUE;
}

BMT_bool _bmt_is_tree_equal(const BMT_node a, const BMT_node b)
{
    return _bmt_is_tree_equal_impl(a, b, 0);
}

BMT_bool _bmt_is_list_equal(const BMT_node a, const BMT_node b)
{
    BMT_node aCursor = a, bCursor = b;
    for (; aCursor && bCursor; aCursor = aCursor->major, bCursor = bCursor->major)
    {
        if (!bmt_IsList(aCursor) || !bmt_IsList(bCursor))
            return BMT_FALSE;

        if (!_bmt_is_equal(aCursor, bCursor))
            return BMT_FALSE;
    }

    return (!aCursor && !bCursor);
}

BMT_bool bmt_IsEqual(const BMT_node a, const BMT_node b)
{
    if (a == b)
        return BMT_TRUE;

    if (bmt_IsTree(a) != bmt_IsTree(b))
        return BMT_FALSE;

    if (bmt_IsList(a) != bmt_IsList(b))
        return BMT_FALSE;

    _bmt_update(a);
    _bmt_update(b);

    return (bmt_IsTree(a) ? _bmt_is_tree_equal(a, b) : _bmt_is_list_equal(a, b));
}

void _bmt_delete_node(BMT_node n)
{
    if (n)
    {
        if (n->name)
            free(n->name);

        switch (n->tag)
        {
            case BMT_TAG_NULL:
                break;
            case BMT_TAG_STRING:
            {
                free(n->payload.tag_string);
                break;
            }

            case BMT_TAG_ROOT:
            case BMT_TAG_LIST:
            case BMT_TAG_ROOT_LIST:
            case BMT_TAG_BYTE_LIST:
            case BMT_TAG_SHORT_LIST:
            case BMT_TAG_INT_LIST:
            case BMT_TAG_LONG_LIST:
            case BMT_TAG_FLOAT_LIST:
            case BMT_TAG_DOUBLE_LIST:
            case BMT_TAG_STRING_LIST:
            {
                bmt_Delete(&n->payload.tag_object);
                break;
            }

            default:
                break;
        }
        free(n);
    }
}

void _bmt_delete_tree_impl_depth(BMT_node tree, int depth)
{
    if (tree)
    {
        BMT_node major = tree->major;
        BMT_node minor = tree->minor;
        _bmt_delete_node(tree);
        _bmt_delete_tree_impl_depth(major, depth + 1);
        _bmt_delete_tree_impl_depth(minor, depth + 1);
    }
}

void _bmt_delete_tree_impl(BMT_node tree)
{
    _bmt_delete_tree_impl_depth(tree, 0);
}

void _bmt_delete_list_impl(BMT_node list)
{
    if (list)
    {
        BMT_node cursor = NULL;
        for (cursor = list; cursor != NULL;)
        {
            BMT_node major = cursor->major;
            _bmt_delete_node(cursor);
            cursor = major;
        }
    }
}

void bmt_Delete(BMT_node* node)
{
    if (node && *node)
    {
        if (!(bmt_IsTree(*node) || bmt_IsList(*node)))
        {
            bmt_error_flag = BMT_STATUS_BAD_VALUE;
            return;
        }

        if (bmt_IsTree(*node))
        {
            _bmt_delete_tree_impl(*node);
        }
        else
        {
            _bmt_delete_list_impl(*node);
        }
        *node = NULL;
    }
}

static BMT_node _bmt_search_impl(const BMT_node tree, const BMT_char* name, int depth)
{
    if (tree)
    {
        BMT_int cmp = _bmt_strcmp(name, tree->name);
        if (cmp == 0)
            return tree;

        if (cmp > 0)
            return _bmt_search_impl(tree->major, name, depth + 1);
        else
            return _bmt_search_impl(tree->minor, name, depth + 1);
    }
    return NULL;
}

BMT_node _bmt_search(const BMT_node tree, const BMT_char* name)
{
    BMT_node ret = NULL;
    if (bmt_IsTree(tree))
    {
        ret = _bmt_search_impl(tree, name, 0);
    }
    return ret;
}

static BMT_node _bmt_insert_data(BMT_node* head,
                                 BMT_node node,
                                 const BMT_char* name,
                                 BMT_tag tag,
                                 BMT_size length,
                                 const void* value,
                                 BMT_bool copyValue)
{
    if (!node)
    {
        *head = _bmt_alloc_node(tag, name, length, value, copyValue);
        if (!*head)
        {
            bmt_error_flag = BMT_STATUS_NO_MEMORY;
        }
        return *head;
    }

    BMT_int cmp;
    if (node->name)
    {
        cmp = _bmt_strcmp(name, node->name);
        if (cmp == 0)
        {
            bmt_error_flag = BMT_STATUS_BAD_NAME;
            return NULL;
        }
    }

    if ((node->nameLength == 0 || node->name == NULL) || node->tag == BMT_TAG_NULL)
    {
        if (!_bmt_copy_payload_from_memory(node, tag, length, value, copyValue))
        {
            // Payload copy failed, error flag already set
            return NULL;
        }
        node->tag = tag;

        node->nameLength = strlen(name);
        _bmt_strncpy(&node->name, name, node->nameLength);

        return *head;
    }

    cmp = _bmt_strcmp(name, node->name);
    BMT_bool maj = cmp > 0;
    BMT_node primary = maj ? node->major : node->minor;

    if (primary)
    {
        _bmt_insert_data(head, primary, name, tag, length, value, copyValue);
    }
    else
    {
        primary = _bmt_alloc_node(tag, name, length, value, copyValue);
        if (!primary)
            return NULL;

        primary->parent = node;
        primary->red = BMT_TRUE;
        maj ? (node->major = primary) : (node->minor = primary);
        _bmt_fix_rbt_violations(primary, head);
    }
    return primary;
}

const BMT_char* _bmt_tag_to_str(BMT_tag tag)
{
    switch (tag)
    {
        case BMT_TAG_ROOT:
            return "tree";
        case BMT_TAG_BYTE:
            return "byte";
        case BMT_TAG_SHORT:
            return "short";
        case BMT_TAG_INT:
            return "int";
        case BMT_TAG_LONG:
            return "long";
        case BMT_TAG_FLOAT:
            return "float";
        case BMT_TAG_DOUBLE:
            return "double";
        case BMT_TAG_STRING:
            return "string";
        case BMT_TAG_LIST:
            return "multi-list";
        case BMT_TAG_ROOT_LIST:
            return "tree-list";
        case BMT_TAG_BYTE_LIST:
            return "byte-list";
        case BMT_TAG_SHORT_LIST:
            return "short-list";
        case BMT_TAG_INT_LIST:
            return "int-list";
        case BMT_TAG_LONG_LIST:
            return "long-list";
        case BMT_TAG_FLOAT_LIST:
            return "float-list";
        case BMT_TAG_DOUBLE_LIST:
            return "double-list";
        case BMT_TAG_STRING_LIST:
            return "string-list";
        default:
            break;
    }
    return "NULL";
}

static void _bmt_bprintf(BMT_buffer* b, const BMT_char* restrict format, ...)
{
    va_list args;
    BMT_size size;

    va_start(args, format);
    size = vsnprintf(NULL, 0, format, args);
    va_end(args);

    if (!bmt_BufferReserve(b, b->length + size + 1))
        return;

    va_start(args, format);
    vsnprintf((char*)(b->data + b->length), size + 1, format, args);
    va_end(args);

    b->length += size;
}

static void _bmt_print(const BMT_node tree, BMT_buffer* buffer, int level, BMT_bool printTreeData)
{
    if (tree)
    {
        BMT_char color = tree->red ? 'R' : 'B';
        BMT_char rank = _bmt_is_root(tree) ? '~' : (_bmt_is_major(tree) ? '+' : '-');
        const BMT_char* format = printTreeData ? "(%c%c) %s (%lld elements) (%lld):\n" : "%s (%lld elements):\n";

        for (int i = 0; i < level; i++)
            _bmt_bprintf(buffer, "\t");

        switch (tree->tag)
        {
            case BMT_TAG_ROOT:
            {
                if (printTreeData)
                    _bmt_bprintf(buffer, "(%c%c) %s (\"%s\"):\n", color, rank, _bmt_tag_to_str(tree->tag), tree->name);
                else
                    _bmt_bprintf(buffer, "%s:\n", _bmt_tag_to_str(tree->tag));

                _bmt_print_tree(tree->payload.tag_object, buffer, level + 1);
                break;
            }

            case BMT_TAG_BYTE:
            {
                if (printTreeData)
                    _bmt_bprintf(buffer,
                                 "(%c%c) %s (\"%s\"): %hhd\n",
                                 color,
                                 rank,
                                 _bmt_tag_to_str(tree->tag),
                                 tree->name,
                                 tree->payload.tag_byte);
                else
                    _bmt_bprintf(buffer, "%s: %hhd\n", _bmt_tag_to_str(tree->tag), tree->payload.tag_byte);

                break;
            }

            case BMT_TAG_SHORT:
            {
                if (printTreeData)
                    _bmt_bprintf(buffer,
                                 "(%c%c) %s (\"%s\"): %hd\n",
                                 color,
                                 rank,
                                 _bmt_tag_to_str(tree->tag),
                                 tree->name,
                                 tree->payload.tag_short);
                else
                    _bmt_bprintf(buffer, "%s: %hd\n", _bmt_tag_to_str(tree->tag), tree->payload.tag_short);
                break;
            }

            case BMT_TAG_INT:
            {
                if (printTreeData)
                    _bmt_bprintf(buffer,
                                 "(%c%c) %s (\"%s\"): %d\n",
                                 color,
                                 rank,
                                 _bmt_tag_to_str(tree->tag),
                                 tree->name,
                                 tree->payload.tag_int);
                else
                    _bmt_bprintf(buffer, "%s: %d\n", _bmt_tag_to_str(tree->tag), tree->payload.tag_int);
                break;
            }

            case BMT_TAG_LONG:
            {
                if (printTreeData)
                    _bmt_bprintf(buffer,
                                 "(%c%c) %s (\"%s\"): %lld\n",
                                 color,
                                 rank,
                                 _bmt_tag_to_str(tree->tag),
                                 tree->name,
                                 tree->payload.tag_long);
                else
                    _bmt_bprintf(buffer, "%s: %lld\n", _bmt_tag_to_str(tree->tag), tree->payload.tag_long);
                break;
            }

            case BMT_TAG_FLOAT:
            {
                if (printTreeData)
                    _bmt_bprintf(buffer,
                                 "(%c%c) %s (\"%s\"): %f\n",
                                 color,
                                 rank,
                                 _bmt_tag_to_str(tree->tag),
                                 tree->name,
                                 tree->payload.tag_float);
                else
                    _bmt_bprintf(buffer, "%s: %f\n", _bmt_tag_to_str(tree->tag), tree->payload.tag_float);
                break;
            }

            case BMT_TAG_DOUBLE:
            {
                if (printTreeData)
                    _bmt_bprintf(buffer,
                                 "(%c%c) %s (\"%s\"): %f\n",
                                 color,
                                 rank,
                                 _bmt_tag_to_str(tree->tag),
                                 tree->name,
                                 tree->payload.tag_double);
                else
                    _bmt_bprintf(buffer, "%s: %f\n", _bmt_tag_to_str(tree->tag), tree->payload.tag_double);
                break;
            }

            case BMT_TAG_STRING:
            {
                if (printTreeData)
                    _bmt_bprintf(buffer,
                                 "(%c%c) %s (\"%s\"): \"%s\"\n",
                                 color,
                                 rank,
                                 _bmt_tag_to_str(tree->tag),
                                 tree->name,
                                 tree->payload.tag_string);
                else
                    _bmt_bprintf(buffer, "%s: %s\n", _bmt_tag_to_str(tree->tag), tree->payload.tag_string);
                break;
            }

            case BMT_TAG_NULL:
            default:
            {
                if (printTreeData)
                    _bmt_bprintf(buffer,
                                 "(%c%c) %s (%lld elements) (\"%s\"):\n",
                                 color,
                                 rank,
                                 _bmt_tag_to_str(tree->tag),
                                 tree->length,
                                 tree->name);
                else
                    _bmt_bprintf(buffer, "%s (%lld elements):\n", _bmt_tag_to_str(tree->tag), tree->length);
                _bmt_print_list(tree->payload.tag_object, buffer, level + 1);
                break;
            }
        }
    }
}

void _bmt_print_list(const BMT_node list, BMT_buffer* buffer, int level)
{
    if (list)
    {
        BMT_node cursor = NULL;
        BMT_FOR_EACH(list, cursor)
        {
            switch (cursor->tag)
            {
                case BMT_TAG_NULL:
                    break;

                case BMT_TAG_ROOT:
                {
                    for (int i = 0; i < level; i++)
                        _bmt_bprintf(buffer, "\t");
                    _bmt_bprintf(buffer, "%s:\n", _bmt_tag_to_str(cursor->tag));
                    _bmt_print_tree(cursor->payload.tag_object, buffer, level + 1);
                    break;
                }

                case BMT_TAG_LIST:
                case BMT_TAG_ROOT_LIST:
                case BMT_TAG_BYTE_LIST:
                case BMT_TAG_SHORT_LIST:
                case BMT_TAG_INT_LIST:
                case BMT_TAG_LONG_LIST:
                case BMT_TAG_FLOAT_LIST:
                case BMT_TAG_DOUBLE_LIST:
                case BMT_TAG_STRING_LIST:
                {
                    for (int i = 0; i < level; i++)
                        _bmt_bprintf(buffer, "\t");

                    _bmt_bprintf(buffer,
                                 "%s (%lld elements):\n",
                                 _bmt_tag_to_str(cursor->tag),
                                 _bmt_length_of_list(cursor->payload.tag_object));
                    _bmt_print_list(cursor->payload.tag_object, buffer, level + 1);
                    break;
                }

                default:
                {
                    _bmt_print(cursor, buffer, level, BMT_FALSE);
                    break;
                }
            }
        }
        _bmt_bprintf(buffer, "\n");
    }
}

void _bmt_print_tree_depth(const BMT_node tree, BMT_buffer* buffer, int level, int depth)
{
    if (tree)
    {
        _bmt_print(tree, buffer, level, BMT_TRUE);
        _bmt_print_tree_depth(tree->major, buffer, level + 1, depth + 1);
        _bmt_print_tree_depth(tree->minor, buffer, level + 1, depth + 1);
    }
}

void _bmt_print_tree(const BMT_node tree, BMT_buffer* buffer, int level)
{
    _bmt_print_tree_depth(tree, buffer, level, 0);
}

BMT_buffer bmt_ToString(const BMT_node node)
{
    BMT_buffer buffer = BMT_BUFFER_INIT;
    if (node)
    {
        if (!(bmt_IsTree(node) || bmt_IsList(node)))
        {
            bmt_error_flag = BMT_STATUS_BAD_VALUE;
            return buffer;
        }

        _bmt_update(node);
        if (bmt_IsTree(node))
        {
            _bmt_print_tree(node, &buffer, 0);
            _bmt_bprintf(&buffer, "\n~ ... Root\n");
            _bmt_bprintf(&buffer, "+ ... Major\n");
            _bmt_bprintf(&buffer, "- ... Minor\n");
            _bmt_bprintf(&buffer, "B ... Black\n");
            _bmt_bprintf(&buffer, "R ... Red\n\n");
            _bmt_bprintf(&buffer, "\n");
        }
        else
        {
            _bmt_print_list(node, &buffer, 0);
        }
    }
    return buffer;
}

void bmt_Print(const BMT_node node)
{
    if (node)
    {
        BMT_buffer buf = bmt_ToString(node);
        printf("%s", (const BMT_char*)buf.data);
        bmt_BufferFree(&buf);
    }
}

void bmt_InsertByte(BMT_node* tree, const BMT_char* name, BMT_byte value)
{
    BMT_CHECK_INPUT(name);
    _bmt_insert_data(tree, *tree, name, BMT_TAG_BYTE, sizeof(BMT_byte), &value, BMT_TRUE);
}

void bmt_InsertShort(BMT_node* tree, const BMT_char* name, BMT_short value)
{
    BMT_CHECK_INPUT(name);
    _bmt_insert_data(tree, *tree, name, BMT_TAG_SHORT, sizeof(BMT_short), &value, BMT_TRUE);
}

void bmt_InsertInt(BMT_node* tree, const BMT_char* name, BMT_int value)
{
    BMT_CHECK_INPUT(name);
    _bmt_insert_data(tree, *tree, name, BMT_TAG_INT, sizeof(BMT_int), &value, BMT_TRUE);
}

void bmt_InsertLong(BMT_node* tree, const BMT_char* name, BMT_long value)
{
    BMT_CHECK_INPUT(name);
    _bmt_insert_data(tree, *tree, name, BMT_TAG_LONG, sizeof(BMT_long), &value, BMT_TRUE);
}

void bmt_InsertFloat(BMT_node* tree, const BMT_char* name, BMT_float value)
{
    BMT_CHECK_INPUT(name);
    _bmt_insert_data(tree, *tree, name, BMT_TAG_FLOAT, sizeof(BMT_float), &value, BMT_TRUE);
}

void bmt_InsertDouble(BMT_node* tree, const BMT_char* name, BMT_double value)
{
    BMT_CHECK_INPUT(name);
    _bmt_insert_data(tree, *tree, name, BMT_TAG_DOUBLE, sizeof(BMT_double), &value, BMT_TRUE);
}

void bmt_InsertString(BMT_node* tree, const BMT_char* name, const BMT_char* value)
{
    if (!value)
    {
        bmt_error_flag = BMT_STATUS_BAD_VALUE;
        return;
    }

    BMT_CHECK_INPUT(name);
    BMT_size valueLength = strlen(value);
    // Prevent excessive string allocation
    if (valueLength >= BMT_MAX_STRING_LENGTH)
    {
        bmt_error_flag = BMT_STATUS_BAD_VALUE;
        return;
    }
    _bmt_insert_data(tree, *tree, name, BMT_TAG_STRING, valueLength, value, BMT_TRUE);
}

static void _bmt_insert_list(BMT_node* tree, const BMT_char* name, BMT_tag tag, BMT_size length, const void* values)
{
    BMT_CHECK_INPUT(name);
    if (!values || length <= 0)
    {
        bmt_error_flag = BMT_STATUS_BAD_VALUE;
        return;
    }

    BMT_node list = bmt_ToList(tag & ~BMT_TAG_LIST, length, values);
    if (!list)
    {
        // Error flag already set by bmt_ToList or its callees
        if (bmt_error_flag == BMT_STATUS_OK)
            bmt_error_flag = BMT_STATUS_NO_MEMORY;
        return;
    }
    _bmt_insert_data(tree, *tree, name, tag, _bmt_length_of_list(list), list, BMT_FALSE);
}

void bmt_InsertByteList(BMT_node* tree, const BMT_char* name, BMT_size length, const BMT_byte* values)
{
    _bmt_insert_list(tree, name, BMT_TAG_BYTE_LIST, length, values);
}

void bmt_InsertShortList(BMT_node* tree, const BMT_char* name, BMT_size length, const BMT_short* values)
{
    _bmt_insert_list(tree, name, BMT_TAG_SHORT_LIST, length, values);
}

void bmt_InsertIntList(BMT_node* tree, const BMT_char* name, BMT_size length, const BMT_int* values)
{
    _bmt_insert_list(tree, name, BMT_TAG_INT_LIST, length, values);
}

void bmt_InsertLongList(BMT_node* tree, const BMT_char* name, BMT_size length, const BMT_long* values)
{
    _bmt_insert_list(tree, name, BMT_TAG_LONG_LIST, length, values);
}

void bmt_InsertFloatList(BMT_node* tree, const BMT_char* name, BMT_size length, const BMT_float* values)
{
    _bmt_insert_list(tree, name, BMT_TAG_FLOAT_LIST, length, values);
}

void bmt_InsertDoubleList(BMT_node* tree, const BMT_char* name, BMT_size length, const BMT_double* values)
{
    _bmt_insert_list(tree, name, BMT_TAG_DOUBLE_LIST, length, values);
}

void bmt_InsertStringList(BMT_node* tree, const BMT_char* name, BMT_size length, const BMT_char** values)
{
    _bmt_insert_list(tree, name, BMT_TAG_STRING_LIST, length, values);
}

void bmt_CreateTree(BMT_node* tree, const BMT_char* name)
{
    BMT_CHECK_INPUT(name);
    _bmt_insert_data(tree, *tree, name, BMT_TAG_ROOT, 0, NULL, BMT_FALSE);
}

void bmt_CreateList(BMT_node* tree, const BMT_char* name)
{
    BMT_CHECK_INPUT(name);
    _bmt_insert_data(tree, *tree, name, BMT_TAG_LIST, 0, NULL, BMT_FALSE);
}

void bmt_Insert(BMT_node* tree, const BMT_char* name, const BMT_node value)
{
    BMT_CHECK_INPUT(name);
    BMT_tag tag = _bmt_get_object_tag(value);

    if (tag == BMT_TAG_NULL)
    {
        bmt_error_flag = BMT_STATUS_BAD_TAG;
        return;
    }

    _bmt_insert_data(tree, *tree, name, tag, bmt_IsList(value) ? _bmt_length_of_list(value) : 0, value, BMT_TRUE);
}

BMT_size _bmt_length_of_list(const BMT_node list)
{
    BMT_size length = 0;
    if (bmt_IsList(list))
    {
        BMT_node cursor = NULL;
        BMT_FOR_EACH(list, cursor)
        length++;
    }
    return length;
}

BMT_long bmt_GetNumber(const BMT_node tree, const BMT_char* name)
{
    BMT_node n = _bmt_search(tree, name);
    if (n)
    {
        switch (n->tag)
        {
            case BMT_TAG_BYTE:
                return n->payload.tag_byte;
            case BMT_TAG_SHORT:
                return n->payload.tag_short;
            case BMT_TAG_INT:
                return n->payload.tag_int;
            case BMT_TAG_LONG:
                return n->payload.tag_long;
            default:
                break;
        }
    }
    return 0LL;
}

BMT_double bmt_GetDecimal(const BMT_node tree, const BMT_char* name)
{
    BMT_node n = _bmt_search(tree, name);
    if (n)
    {
        switch (n->tag)
        {
            case BMT_TAG_FLOAT:
                return n->payload.tag_float;
            case BMT_TAG_DOUBLE:
                return n->payload.tag_double;
            default:
                break;
        }
    }
    return 0.0;
}

BMT_byte bmt_GetByte(const BMT_node tree, const BMT_char* name)
{
    BMT_node n = _bmt_search(tree, name);
    return (n && n->tag == BMT_TAG_BYTE) ? n->payload.tag_byte : 0;
}

BMT_short bmt_GetShort(const BMT_node tree, const BMT_char* name)
{
    BMT_node n = _bmt_search(tree, name);
    return (n && n->tag == BMT_TAG_SHORT) ? n->payload.tag_short : 0;
}

BMT_int bmt_GetInt(const BMT_node tree, const BMT_char* name)
{
    BMT_node n = _bmt_search(tree, name);
    return (n && n->tag == BMT_TAG_INT) ? n->payload.tag_int : 0;
}

BMT_long bmt_GetLong(const BMT_node tree, const BMT_char* name)
{
    BMT_node n = _bmt_search(tree, name);
    return (n && n->tag == BMT_TAG_LONG) ? n->payload.tag_long : 0LL;
}

BMT_float bmt_GetFloat(const BMT_node tree, const BMT_char* name)
{
    BMT_node n = _bmt_search(tree, name);
    return (n && n->tag == BMT_TAG_FLOAT) ? n->payload.tag_float : 0.f;
}

BMT_double bmt_GetDouble(const BMT_node tree, const BMT_char* name)
{
    BMT_node n = _bmt_search(tree, name);
    return (n && n->tag == BMT_TAG_DOUBLE) ? n->payload.tag_double : 0.0;
}

const BMT_char* bmt_GetString(const BMT_node tree, const BMT_char* name)
{
    BMT_node n = _bmt_search(tree, name);
    return (n && n->tag == BMT_TAG_STRING) ? n->payload.tag_string : NULL;
}

BMT_node bmt_Get(const BMT_node tree, const BMT_char* name)
{
    return _bmt_search(tree, name);
}

static void _bmt_set_value(BMT_node tree, BMT_tag tag, const BMT_char* name, const void* value)
{
    BMT_node n = _bmt_search(tree, name);
    if (n && n->tag == tag)
    {
        switch (tag)
        {
            case BMT_TAG_BYTE:
                n->payload.tag_byte = *((const BMT_byte*)value);
                break;
            case BMT_TAG_SHORT:
                n->payload.tag_short = *((const BMT_short*)value);
                break;
            case BMT_TAG_INT:
                n->payload.tag_int = *((const BMT_int*)value);
                break;
            case BMT_TAG_LONG:
                n->payload.tag_long = *((const BMT_long*)value);
                break;
            case BMT_TAG_FLOAT:
                n->payload.tag_float = *((const BMT_float*)value);
                break;
            case BMT_TAG_DOUBLE:
                n->payload.tag_double = *((const BMT_double*)value);
                break;
            case BMT_TAG_STRING:
            {
                const BMT_char* newValue = (const BMT_char*)value;
                if (newValue)
                {
                    BMT_size length = strlen(newValue) + 1;
                    // Bounds check
                    if (length - 1 >= BMT_MAX_STRING_LENGTH)
                    {
                        bmt_error_flag = BMT_STATUS_BAD_VALUE;
                        return;
                    }
                    BMT_char* newStr = realloc(n->payload.tag_string, length);
                    if (!newStr)
                    {
                        // Don't free old pointer - leave node in valid state
                        bmt_error_flag = BMT_STATUS_NO_MEMORY;
                        return;
                    }
                    n->payload.tag_string = newStr;
                    memcpy(n->payload.tag_string, newValue, length - 1);
                    n->payload.tag_string[length - 1] = 0;
                    n->length = length - 1;
                }
                break;
            }
            default:
                break;
        }
    }
}

void bmt_SetByte(BMT_node tree, const BMT_char* name, BMT_byte value)
{
    _bmt_set_value(tree, BMT_TAG_BYTE, name, &value);
}

void bmt_SetShort(BMT_node tree, const BMT_char* name, BMT_short value)
{
    _bmt_set_value(tree, BMT_TAG_SHORT, name, &value);
}

void bmt_SetInt(BMT_node tree, const BMT_char* name, BMT_int value)
{
    _bmt_set_value(tree, BMT_TAG_INT, name, &value);
}

void bmt_SetLong(BMT_node tree, const BMT_char* name, BMT_long value)
{
    _bmt_set_value(tree, BMT_TAG_LONG, name, &value);
}

void bmt_SetFloat(BMT_node tree, const BMT_char* name, BMT_float value)
{
    _bmt_set_value(tree, BMT_TAG_FLOAT, name, &value);
}

void bmt_SetDouble(BMT_node tree, const BMT_char* name, BMT_double value)
{
    _bmt_set_value(tree, BMT_TAG_DOUBLE, name, &value);
}

void bmt_SetString(BMT_node tree, const BMT_char* name, const BMT_char* value)
{
    _bmt_set_value(tree, BMT_TAG_STRING, name, value);
}

BMT_node bmt_ToList(BMT_tag tag, BMT_size length, const void* data)
{
    if (!data)
    {
        bmt_error_flag = BMT_STATUS_BAD_VALUE;
        return NULL;
    }

    if (length < 0 || length > BMT_MAX_LIST_LENGTH)
    {
        bmt_error_flag = BMT_STATUS_BAD_VALUE;
        return NULL;
    }

    BMT_node list = NULL;
    for (BMT_size i = 0; i < length; i++)
    {
        switch (tag & ~BMT_TAG_LIST)
        {
            case BMT_TAG_BYTE:
                bmt_AppendByte(&list, *(((const BMT_byte*)data) + i));
                break;
            case BMT_TAG_SHORT:
                bmt_AppendShort(&list, *(((const BMT_short*)data) + i));
                break;
            case BMT_TAG_INT:
                bmt_AppendInt(&list, *(((const BMT_int*)data) + i));
                break;
            case BMT_TAG_LONG:
                bmt_AppendLong(&list, *(((const BMT_long*)data) + i));
                break;
            case BMT_TAG_FLOAT:
                bmt_AppendFloat(&list, *(((const BMT_float*)data) + i));
                break;
            case BMT_TAG_DOUBLE:
                bmt_AppendDouble(&list, *(((const BMT_double*)data) + i));
                break;
            case BMT_TAG_STRING:
            {
                if (*(((const BMT_char**)data) + i))
                    bmt_AppendString(&list, *(((const BMT_char**)data) + i));
                break;
            }
        }

        // Check for allocation errors during append
        if (bmt_error_flag != BMT_STATUS_OK)
        {
            if (list)
                bmt_Delete(&list);
            return NULL;
        }
    }
    return list;
}

BMT_size bmt_Length(const BMT_node node)
{
    return _bmt_length_of_list(node);
}

void bmt_RemoveAt(BMT_node* list, BMT_index pos)
{
    if (!list)
    {
        bmt_error_flag = BMT_STATUS_BAD_VALUE;
        return;
    }

    if (!bmt_IsList(*list))
    {
        bmt_error_flag = BMT_STATUS_NOT_A_LIST;
        return;
    }

    if (pos < 0 || pos >= _bmt_length_of_list(*list))
    {
        bmt_error_flag = BMT_STATUS_BAD_VALUE;
        return;
    }

    BMT_node cursor = *list;
    for (BMT_size i = 1; i <= pos; i++)
    {
        if (!cursor)
        {
            bmt_error_flag = BMT_STATUS_BAD_VALUE;
            return;
        }
        cursor = cursor->major;
    }

    if (cursor)
    {
        BMT_node next = cursor->major;
        BMT_node last = cursor->minor;

        _bmt_delete_node(cursor);

        if (last)
            last->major = next;
        if (next)
            next->minor = last;

        if (pos == 0)
            *list = next;
    }
}

void _bmt_insert_multi_list(BMT_node* _list, BMT_tag tag, BMT_size length, const void* values, BMT_bool copyValue)
{
    if (!_list)
    {
        bmt_error_flag = BMT_STATUS_BAD_VALUE;
        return;
    }

    BMT_node list = *_list;
    if (!list)
    {
        *_list = bmt_AllocList();
        if (!*_list)
        {
            bmt_error_flag = BMT_STATUS_NO_MEMORY;
            return;
        }
        (*_list)->tag = tag;
        if (!_bmt_copy_payload_from_memory(*_list, tag, length, values, copyValue))
        {
            bmt_Delete(_list);
            return;
        }
    }
    else
    {
        if (!bmt_IsList(list))
        {
            bmt_error_flag = BMT_STATUS_NOT_A_LIST;
            return;
        }

        if (list->tag == BMT_TAG_NULL)
        {
            _bmt_copy_payload_from_memory(list, tag, length, values, copyValue);
            list->tag = tag;
        }
        else
        {
            if (list->tag == tag)
            {
                BMT_node cursor = list;
                while (cursor->major)
                    cursor = cursor->major;

                cursor->major = bmt_AllocList();
                if (!cursor->major)
                {
                    bmt_error_flag = BMT_STATUS_NO_MEMORY;
                    return;
                }
                cursor->major->tag = tag;
                cursor->major->minor = cursor;
                if (!_bmt_copy_payload_from_memory(cursor->major, tag, length, values, copyValue))
                {
                    free(cursor->major);
                    cursor->major = NULL;
                    return;
                }
            }
        }
    }
}

void bmt_AppendByte(BMT_node* list, BMT_byte value)
{
    _bmt_insert_multi_list(list, BMT_TAG_BYTE, sizeof(BMT_byte), &value, BMT_TRUE);
}

void bmt_AppendShort(BMT_node* list, BMT_short value)
{
    _bmt_insert_multi_list(list, BMT_TAG_SHORT, sizeof(BMT_short), &value, BMT_TRUE);
}

void bmt_AppendInt(BMT_node* list, BMT_int value)
{
    _bmt_insert_multi_list(list, BMT_TAG_INT, sizeof(BMT_int), &value, BMT_TRUE);
}

void bmt_AppendLong(BMT_node* list, BMT_long value)
{
    _bmt_insert_multi_list(list, BMT_TAG_LONG, sizeof(BMT_long), &value, BMT_TRUE);
}

void bmt_AppendFloat(BMT_node* list, BMT_float value)
{
    _bmt_insert_multi_list(list, BMT_TAG_FLOAT, sizeof(BMT_float), &value, BMT_TRUE);
}

void bmt_AppendDouble(BMT_node* list, BMT_double value)
{
    _bmt_insert_multi_list(list, BMT_TAG_DOUBLE, sizeof(BMT_double), &value, BMT_TRUE);
}

void bmt_AppendString(BMT_node* list, const BMT_char* value)
{
    if (!value)
    {
        bmt_error_flag = BMT_STATUS_BAD_VALUE;
        return;
    }
    _bmt_insert_multi_list(list, BMT_TAG_STRING, strlen(value), value, BMT_TRUE);
}

void bmt_AppendByteList(BMT_node* list, BMT_size length, const BMT_byte* values)
{
    BMT_node v = bmt_ToList(BMT_TAG_BYTE, length, values);
    if (!v)
    {
        if (bmt_error_flag == BMT_STATUS_OK)
            bmt_error_flag = BMT_STATUS_NO_MEMORY;
        return;
    }
    _bmt_insert_multi_list(list, BMT_TAG_BYTE_LIST, length, v, BMT_FALSE);
}

void bmt_AppendShortList(BMT_node* list, BMT_size length, const BMT_short* values)
{
    BMT_node v = bmt_ToList(BMT_TAG_SHORT, length, values);
    if (!v)
    {
        if (bmt_error_flag == BMT_STATUS_OK)
            bmt_error_flag = BMT_STATUS_NO_MEMORY;
        return;
    }
    _bmt_insert_multi_list(list, BMT_TAG_SHORT_LIST, length, v, BMT_FALSE);
}

void bmt_AppendIntList(BMT_node* list, BMT_size length, const BMT_int* values)
{
    BMT_node v = bmt_ToList(BMT_TAG_INT, length, values);
    if (!v)
    {
        if (bmt_error_flag == BMT_STATUS_OK)
            bmt_error_flag = BMT_STATUS_NO_MEMORY;
        return;
    }
    _bmt_insert_multi_list(list, BMT_TAG_INT_LIST, length, v, BMT_FALSE);
}

void bmt_AppendLongList(BMT_node* list, BMT_size length, const BMT_long* values)
{
    BMT_node v = bmt_ToList(BMT_TAG_LONG, length, values);
    if (!v)
    {
        if (bmt_error_flag == BMT_STATUS_OK)
            bmt_error_flag = BMT_STATUS_NO_MEMORY;
        return;
    }
    _bmt_insert_multi_list(list, BMT_TAG_LONG_LIST, length, v, BMT_FALSE);
}

void bmt_AppendFloatList(BMT_node* list, BMT_size length, const BMT_float* values)
{
    BMT_node v = bmt_ToList(BMT_TAG_FLOAT, length, values);
    if (!v)
    {
        if (bmt_error_flag == BMT_STATUS_OK)
            bmt_error_flag = BMT_STATUS_NO_MEMORY;
        return;
    }
    _bmt_insert_multi_list(list, BMT_TAG_FLOAT_LIST, length, v, BMT_FALSE);
}

void bmt_AppendDoubleList(BMT_node* list, BMT_size length, const BMT_double* values)
{
    BMT_node v = bmt_ToList(BMT_TAG_DOUBLE, length, values);
    if (!v)
    {
        if (bmt_error_flag == BMT_STATUS_OK)
            bmt_error_flag = BMT_STATUS_NO_MEMORY;
        return;
    }
    _bmt_insert_multi_list(list, BMT_TAG_DOUBLE_LIST, length, v, BMT_FALSE);
}

void bmt_AppendStringList(BMT_node* list, BMT_size length, const BMT_char** values)
{
    BMT_node v = bmt_ToList(BMT_TAG_STRING, length, values);
    if (!v)
    {
        if (bmt_error_flag == BMT_STATUS_OK)
            bmt_error_flag = BMT_STATUS_NO_MEMORY;
        return;
    }
    _bmt_insert_multi_list(list, BMT_TAG_STRING_LIST, _bmt_length_of_list(v), v, BMT_FALSE);
}

void bmt_Append(BMT_node* list, const BMT_node value)
{
    BMT_tag tag = _bmt_get_object_tag(value);
    if (tag == BMT_TAG_NULL)
    {
        bmt_error_flag = BMT_STATUS_BAD_TAG;
        return;
    }
    _bmt_insert_multi_list(list, tag, bmt_IsTree(value) ? 0 : _bmt_length_of_list(value), value, BMT_TRUE);
}

BMT_bool bmt_Remove(BMT_node* tree, const BMT_char* name)
{
    if (!tree)
    {
        bmt_error_flag = BMT_STATUS_BAD_VALUE;
        return BMT_FALSE;
    }

    if (!bmt_IsTree(*tree))
    {
        bmt_error_flag = BMT_STATUS_NOT_A_TREE;
        return BMT_FALSE;
    }

    BMT_node n = _bmt_search(*tree, name);
    if (n)
    {
        BMT_bool red = n->red;
        BMT_node r = NULL;
        BMT_node x = NULL;
        BMT_node w = NULL;
        _bmt_bst_delete_impl(n, &r, &x, &w, tree);
        return _bmt_fix_up_rbt(red, r, x, w, tree);
    }

    return BMT_FALSE;
}

BMT_status bmt_GetLastError()
{
    BMT_status status = bmt_error_flag;
    bmt_error_flag = BMT_STATUS_OK;
    return status;
}

const BMT_char* bmt_GetErrorInfo(BMT_status status)
{
    switch (status)
    {
        case BMT_STATUS_NO_MEMORY:
            return "Failed to allocate memory";
        case BMT_STATUS_READ_ERROR:
            return "Error while reading object";
        case BMT_STATUS_WRITE_ERROR:
            return "Error while writing object";
        case BMT_STATUS_BAD_NAME:
            return "Name was empty or already taken";
        case BMT_STATUS_BAD_VALUE:
            return "Given value was invalid";
        case BMT_STATUS_BAD_TAG:
            return "Given node had non-matching tag";
        case BMT_STATUS_NOT_A_TREE:
            return "Given node was not a tree-node";
        case BMT_STATUS_NOT_A_LIST:
            return "Given node was not a list-node";
        default:
            break;
    }
    return "Ok";
}
#if defined(BMT_COMPILER_CLANG) || defined(BMT_COMPILER_GNUC)
#pragma GCC diagnostic pop
#endif
