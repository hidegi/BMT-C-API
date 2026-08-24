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
#ifndef BMT_INTERNAL_IMPL_H
#define BMT_INTERNAL_IMPL_H
#include "RB/io/bmt.h"

/* Size limits for BMT serialization (bytes) */
#define BMT_MAX_FILE_SIZE (100 * 1024 * 1024)         /* 100 MB - file I/O limit */
#define BMT_MAX_BUFFER_SIZE (1024 * 1024 * 1024)      /* 1 GB - buffer allocation limit */
#define BMT_MAX_DECOMPRESSED_SIZE (256 * 1024 * 1024) /* 256 MB - decompression limit (zip bomb protection) */
#define BMT_MAX_STRING_LENGTH (10 * 1024 * 1024)      /* 10 MB - maximum string length */
#define BMT_MAX_LIST_LENGTH (100 * 1000 * 1000)       /* 100 M elements - maximum list size (memory exhaustion protection) */

#define BMT_CHECKED_CALLOC(ptr, n, size, on_error) \
    do                                             \
    {                                              \
        if (!((ptr) = calloc(n, size)))            \
        {                                          \
            bmt_error_flag = BMT_STATUS_NO_MEMORY; \
            on_error;                              \
        }                                          \
    } while (0)

void _bmt_update(BMT_node node);
void _bmt_update_tree(BMT_node node);
void _bmt_update_list(BMT_node node);
const SPchar* _bmt_tag_to_str(BMT_tag tag);
BMT_node _bmt_search(const BMT_node tree, const SPchar* name);
SPbool _bmt_is_name_valid(const SPchar* name);
SPint _bmt_strcmp(const SPchar* a, const SPchar* b);
void _bmt_strncpy(SPchar** dst, const SPchar* src, SPsize length);
SPbuffer _bmt_compress(const void* memory, SPsize length);
SPbuffer _bmt_decompress(const void* memory, SPsize length);
void _bmt_encode_tree(const BMT_node tree, SPbuffer* buffer, int level);
void _bmt_encode_list(const BMT_node list, SPbuffer* buffer, int level);
BMT_node _bmt_decode_tree(const SPubyte** memory, SPsize* length);
SPbool _bmt_decode_list(BMT_node node, const SPubyte** memory, SPsize* length);
SPbool _bmt_is_tree_equal(const BMT_node a, const BMT_node b);
SPbool _bmt_is_list_equal(const BMT_node a, const BMT_node b);
SPbool _bmt_is_root(const BMT_node node);
SPbool _bmt_is_major(const BMT_node node);
void _bmt_delete_tree_impl(BMT_node tree);
void _bmt_delete_list_impl(BMT_node tree);
void _bmt_fix_rbt_violations(BMT_node node, BMT_node* head);
void _bmt_delete_node(BMT_node n);
void _bmt_bst_delete_impl(BMT_node n, BMT_node* _r, BMT_node* _x, BMT_node* _w, BMT_node* head);
SPbool _bmt_fix_up_rbt(SPbool redBefore, BMT_node r, BMT_node x, BMT_node w, BMT_node* head);
void _bmt_print_tree(const BMT_node tree, SPbuffer* buffer, int level);
void _bmt_print_list(const BMT_node tree, SPbuffer* buffer, int level);
SPsize _bmt_length_of_list(const BMT_node list);
#endif
