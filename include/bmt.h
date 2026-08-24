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
#ifndef BMT_PUBLIC_INTERFACE_H
#define BMT_PUBLIC_INTERFACE_H
#if defined(_MSC_VER)
#define BMT_THREAD_LOCAL __declspec(thread)
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
// C11 standard
#define BMT_THREAD_LOCAL _Thread_local
#else
// Fallback for GCC/Clang
#define BMT_THREAD_LOCAL __thread
#endif

#include "buffer.h"
#include "types.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        /// Plain-type tags.
        BMT_TAG_NULL = 0,
        BMT_TAG_ROOT = 1,
        BMT_TAG_BYTE = 2,
        BMT_TAG_SHORT = 3,
        BMT_TAG_INT = 4,
        BMT_TAG_LONG = 5,
        BMT_TAG_FLOAT = 6,
        BMT_TAG_DOUBLE = 7,
        BMT_TAG_STRING = 8,
        BMT_TAG_MAX = BMT_TAG_STRING,

        BMT_TAG_LIST = 0x80,

        /// List-type tags.
        BMT_TAG_ROOT_LIST = BMT_TAG_LIST | BMT_TAG_ROOT,
        BMT_TAG_BYTE_LIST = BMT_TAG_LIST | BMT_TAG_BYTE,
        BMT_TAG_SHORT_LIST = BMT_TAG_LIST | BMT_TAG_SHORT,
        BMT_TAG_INT_LIST = BMT_TAG_LIST | BMT_TAG_INT,
        BMT_TAG_LONG_LIST = BMT_TAG_LIST | BMT_TAG_LONG,
        BMT_TAG_FLOAT_LIST = BMT_TAG_LIST | BMT_TAG_FLOAT,
        BMT_TAG_DOUBLE_LIST = BMT_TAG_LIST | BMT_TAG_DOUBLE,
        BMT_TAG_STRING_LIST = BMT_TAG_LIST | BMT_TAG_STRING
    } BMT_tag;

    typedef enum
    {
        BMT_STATUS_OK = 0,      /// No errors.
        BMT_STATUS_NO_MEMORY,   /// Out of memory.
        BMT_STATUS_READ_ERROR,  /// Read error.
        BMT_STATUS_WRITE_ERROR, /// Write error.
        BMT_STATUS_BAD_NAME,    /// Bad name, either taken, empty or null.
        BMT_STATUS_BAD_VALUE,   /// Invalid value.
        BMT_STATUS_NOT_A_TREE,  /// Given node is not a tree.
        BMT_STATUS_NOT_A_LIST,  /// Given node is not a list.
        BMT_STATUS_BAD_TAG,     /// Invalid tag.
    } BMT_status;

    /**
     * Thread-local error storage for BMT operations.
     *
     * Each thread has its own error flag, making BMT functions thread-safe
     * with respect to error reporting. After calling any BMT function,
     * check for errors using bmt_GetLastError() which clears the flag.
     *
     * Note: This is thread-local, not instance-local. If you use BMT from
     * multiple threads operating on different trees, each thread will have
     * its own error state. This is intentional for thread-safety.
     */
    extern BMT_THREAD_LOCAL BMT_status bmt_error_flag;

    struct _BMT_node
    {
            // Do not modify these members directly!
            // (Causes undefined behaviour)
            BMT_tag tag;

            BMT_ubyte nameLength; // Name should not exceed 255 characters.
            BMT_char* name;       // The name of the node.

            BMT_size length; // The length of the payload in bytes.

            BMT_bool red; // Signals the node's color, encoded in the 7th bit of the tag.
            struct _BMT_node* parent;
            struct _BMT_node* major;
            struct _BMT_node* minor;

            union
            {
                    // Payload permitted for direct modification, except
                    // tag_string and tag_object.
                    BMT_byte tag_byte;
                    BMT_short tag_short;
                    BMT_int tag_int;
                    BMT_long tag_long;
                    BMT_float tag_float;
                    BMT_double tag_double;
                    BMT_char* tag_string;
                    struct _BMT_node* tag_object; // For all list-types and trees.
            } payload;
    };
    typedef struct _BMT_node* BMT_node;

    /*!
     *	@brief Allocates a tree-node.
     *
     *	The returned node will be black-coded and
     *	will have a tag of BMT_TAG_NULL.
     *
     *	@return The newly allocated tree, otherwise NULL,
     *	if failing to reserve memory.
     *
     *	@ingroup allocation
     */
    BMT_node bmt_AllocTree();

    /*!
     *	@brief Allocates a list-node.
     *
     *	The returned node will be red-coded and
     *	will have a tag of BMT_TAG_NULL.
     *
     *	@return The newly allocated list, otherwise NULL,
     *	if failing to reserve memory.
     *
     *	@ingroup allocation
     */
    BMT_node bmt_AllocList();

    /*!
     *	@brief Dumps an object to a human-readable string.
     *
     *	For tree-nodes, the node will be shown with either R or B
     *	for red or black respectively.
     *
     *	A plus '+' shows the major branch, a minus '-' the minor branch,
     *	a tilde '~' shows a root node.
     *
     *	For list-nodes, each element will be shown line by line.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE,
     *	if @param object is not a head-node.
     *
     *	@return An BMT_buffer containing the output string. Caller must call
     *	        bmt_BufferFree() on the returned buffer when done.
     *	        DO NOT use this in performance-critical code due to allocation overhead.
     *
     *	@ingroup debug
     */
    BMT_buffer bmt_ToString(const BMT_node object);

    /*!
     *	@brief Same as @ref bmt_ToString, but prints the output
     *	to the standard output stream.
     *
     *	@ingroup debug
     */
    void bmt_Print(const BMT_node object);

    /*!
     *	@brief Compares the tag, colour, value and
     *  weight between two nodes.
     *
     *	@return True, if @param a is virtually
     *  equal to @param b.
     *
     *	@ingroup comparison
     */
    BMT_bool bmt_IsEqual(const BMT_node a, const BMT_node b);

    /*!
     *	@brief Copies a tree-or list-node.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE,
     *	if @param object is not a head-node.
     *
     *	@return The newly copied node.
     *
     *	@ingroup copying
     */
    BMT_node bmt_Copy(const BMT_node object);

    /*!
     *	@brief Tells whether or not @param tree is a tree-node.
     *	A tree is defined to be black-coded without a parent.
     *
     *	Expects @param object to be a head-node, otherwise will
     *	always return false.
     *
     *	@return True, if @param node is a tree-node.
     *
     *	@ingroup tree-validation
     */
    BMT_bool bmt_IsTree(const BMT_node tree);

    /*!
     *	@brief Inserts an empty tree-node to the given tree.
     *	If @param tree points to NULL, a new tree will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param tree is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param tree is not NULL and not a tree.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	@ingroup tree-insertion
     */
    void bmt_CreateTree(BMT_node* tree, const BMT_char* name);

    /*!
     *	@brief Inserts an empty list-node to the given tree.
     *	If @param tree points to NULL, a new tree will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param tree is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param tree is not NULL and not a tree.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	@ingroup tree-insertion
     */
    void bmt_CreateList(BMT_node* tree, const BMT_char* name);

    /*!
     *	@brief Inserts a tree-or list-node to the given tree.
     *	If @param tree points to NULL, a new tree will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param tree is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param tree is not NULL and not a tree.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	@ingroup tree-insertion
     */
    void bmt_Insert(BMT_node* tree, const BMT_char* name, const BMT_node value);

    /*!
     *	@brief Inserts a byte-node to the given tree.
     *	If @param tree points to NULL, a new tree will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param tree is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param tree is not NULL and not a tree.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	@ingroup tree-insertion
     */
    void bmt_InsertByte(BMT_node* tree, const BMT_char* name, BMT_byte value);

    /*!
     *	@brief Inserts a short-node to the given tree.
     *	If @param tree points to NULL, a new tree will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param tree is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param tree is not NULL and not a tree.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	@ingroup tree-insertion
     */
    void bmt_InsertShort(BMT_node* tree, const BMT_char* name, BMT_short value);

    /*!
     *	@brief Inserts an int-node to the given tree.
     *	If @param tree points to NULL, a new tree will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param tree is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param tree is not NULL and not a tree.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	@ingroup tree-insertion
     */
    void bmt_InsertInt(BMT_node* tree, const BMT_char* name, BMT_int value);

    /*!
     *	@brief Inserts a long-node to the given tree.
     *	If @param tree points to NULL, a new tree will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param tree is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param tree is not NULL and not a tree.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	@ingroup tree-insertion
     */
    void bmt_InsertLong(BMT_node* tree, const BMT_char* name, BMT_long value);

    /*!
     *	@brief Inserts a float-node to the given tree.
     *	If @param tree points to NULL, a new tree will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param tree is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param tree is not NULL and not a tree.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	@ingroup tree-insertion
     */
    void bmt_InsertFloat(BMT_node* tree, const BMT_char* name, BMT_float value);

    /*!
     *	@brief Inserts a double-node to the given tree.
     *	If @param tree points to NULL, a new tree will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param tree is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param tree is not NULL and not a tree.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	@ingroup tree-insertion
     */
    void bmt_InsertDouble(BMT_node* tree, const BMT_char* name, BMT_double value);

    /*!
     *	@brief Inserts a string-node to the given tree.
     *	If @param tree points to NULL, a new tree will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param tree is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param tree is not NULL and not a tree.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param value is NULL.
     *
     *	@ingroup tree-insertion
     */
    void bmt_InsertString(BMT_node* tree, const BMT_char* name, const BMT_char* value);

    /*!
     *	@brief Inserts a byte-list-node to the given tree.
     *	If @param tree points to NULL, a new tree will be created.
     *	A linked list will be implicitly created from @param values.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param tree is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param tree is not NULL and not a tree.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param value is NULL,
     *	or @param length is less-equal to zero.
     *
     *	@ingroup tree-insertion
     */
    void bmt_InsertByteList(BMT_node* tree, const BMT_char* name, BMT_size length, const BMT_byte* values);

    /*!
     *	@brief Inserts a short-list-node to the given tree.
     *	If @param tree points to NULL, a new tree will be created.
     *	A linked list will be implicitly created from @param values.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param tree is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param tree is not NULL and not a tree.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param value is NULL,
     *	or @param length is less-equal to zero.
     *
     *	@ingroup tree-insertion
     */
    void bmt_InsertShortList(BMT_node* tree, const BMT_char* name, BMT_size length, const BMT_short* values);

    /*!
     *	@brief Inserts an int-list-node to the given tree.
     *	If @param tree points to NULL, a new tree will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param tree is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param tree is not NULL and not a tree.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param value is NULL,
     *	or @param length is less-equal to zero.
     *
     *	@ingroup tree-insertion
     */
    void bmt_InsertIntList(BMT_node* tree, const BMT_char* name, BMT_size length, const BMT_int* values);

    /*!
     *	@brief Inserts a long-list-node to the given tree.
     *	If @param tree points to NULL, a new tree will be created.
     *	A linked list will be implicitly created from @param values.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param tree is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param tree is not NULL and not a tree.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param value is NULL,
     *	or @param length is less-equal to zero.
     *
     *	@ingroup tree-insertion
     */
    void bmt_InsertLongList(BMT_node* tree, const BMT_char* name, BMT_size length, const BMT_long* values);

    /*!
     *	@brief Inserts a float-list-node to the given tree.
     *	If @param tree points to NULL, a new tree will be created.
     *	A linked list will be implicitly created from @param values.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param tree is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param tree is not NULL and not a tree.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param value is NULL,
     *	or @param length is less-equal to zero.
     *
     *	@ingroup tree-insertion
     */
    void bmt_InsertFloatList(BMT_node* tree, const BMT_char* name, BMT_size length, const BMT_float* values);

    /*!
     *	@brief Inserts a double-list-node to the given tree.
     *	If @param tree points to NULL, a new tree will be created.
     *	A linked list will be implicitly created from @param values.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param tree is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param tree is not NULL and not a tree.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param value is NULL,
     *	or @param length is less-equal to zero.
     *
     *	@ingroup tree-insertion
     */
    void bmt_InsertDoubleList(BMT_node* tree, const BMT_char* name, BMT_size length, const BMT_double* values);

    /*!
     *	@brief Inserts a string-list-node to the given tree.
     *	If @param tree points to NULL, a new tree will be created.
     *	A linked list will be implicitly created from @param values.
     *	Note that NULL-strings will be ignored.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param tree is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param tree is not NULL and not a tree.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param value is NULL,
     *	or @param length is less-equal to zero.
     *
     *	@ingroup tree-insertion
     */
    void bmt_InsertStringList(BMT_node* tree, const BMT_char* name, BMT_size length, const BMT_char** values);

    /*!
     *	@brief Returns the value stored in any integer-type nodes
     *	found by @param name.
     *
     *	@return Returns 0LL, if @param tree is not a tree or
     *	the node labeled by @param name is not available,
     *	otherwise the value stored in the node labeled by
     *	@param name.
     *
     *	@ingroup tree-getters
     */
    BMT_long bmt_GetNumber(const BMT_node tree, const BMT_char* name);

    /*!
     *	@brief Returns the value stored in any floating-type nodes
     *	found by @param name.
     *
     *	@return Returns 0.0, if @param tree is not a tree or
     *	the node labeled by @param name is not available,
     *	otherwise the value stored in the node labeled by
     *	@param name.
     *
     *	@ingroup tree-getters
     */
    BMT_double bmt_GetDecimal(const BMT_node tree, const BMT_char* name);

    /*!
     *	@brief Returns the value stored in a byte-node
     *	found by @param name.
     *
     *	@return Returns 0, if @param tree is not a tree or
     *	the node labeled by @param name is not available,
     *	otherwise the value stored in the node labeled by
     *	@param name.
     *
     *	@ingroup tree-getters
     */
    BMT_byte bmt_GetByte(const BMT_node tree, const BMT_char* name);

    /*!
     *	@brief Returns the value stored in a short-node
     *	found by @param name.
     *
     *	@return Returns 0, if @param tree is not a tree or
     *	the node labeled by @param name is not available,
     *	otherwise the value stored in the node labeled by
     *	@param name.
     *
     *	@ingroup tree-getters
     */
    BMT_short bmt_GetShort(const BMT_node tree, const BMT_char* name);

    /*!
     *	@brief Returns the value stored in an int-node
     *	found by @param name.
     *
     *	@return Returns 0, if @param tree is not a tree or
     *	the node labeled by @param name is not available,
     *	otherwise the value stored in the node labeled by
     *	@param name.
     *
     *	@ingroup tree-getters
     */
    BMT_int bmt_GetInt(const BMT_node tree, const BMT_char* name);

    /*!
     *	@brief Returns the value stored in a long-node
     *	found by @param name.
     *
     *	@return Returns 0LL, if @param tree is not a tree or
     *	the node labeled by @param name is not available,
     *	otherwise the value stored in the node labeled by
     *	@param name.
     *
     *	@ingroup tree-getters
     */
    BMT_long bmt_GetLong(const BMT_node tree, const BMT_char* name);

    /*!
     *	@brief Returns the value stored in a float-node
     *	found by @param name.
     *
     *	@return Returns 0.0, if @param tree is not a tree or
     *	the node labeled by @param name is not available,
     *	otherwise the value stored in the node labeled by
     *	@param name.
     *
     *	@ingroup tree-getters
     */
    BMT_float bmt_GetFloat(const BMT_node tree, const BMT_char* name);

    /*!
     *	@brief Returns the value stored in a double-node
     *	found by @param name.
     *
     *	@return Returns 0.0, if @param tree is not a tree or
     *	the node labeled by @param name is not available,
     *	otherwise the value stored in the node labeled by
     *	@param name.
     *
     *	@ingroup tree-getters
     */
    BMT_double bmt_GetDouble(const BMT_node tree, const BMT_char* name);

    /*!
     *	@brief Returns the value stored in a string-node
     *	found by @param name.
     *
     *	@return Returns NULL, if @param tree is not a tree or
     *	the node labeled by @param name is not available,
     *	otherwise the value stored in the node labeled by
     *	@param name.
     *
     *	@ingroup tree-getters
     */
    const BMT_char* bmt_GetString(const BMT_node tree, const BMT_char* name);

    /*!
     *	@brief Returns the object stored in a tree-or list-node.
     *	found by @param name.
     *
     *	@return Returns NULL, if @param tree is not a tree or
     *	the node labeled by @param name is not available,
     *	otherwise the object stored in the node labeled by
     *	@param name.
     *
     *	@ingroup tree-getters
     */
    BMT_node bmt_Get(const BMT_node tree, const BMT_char* name);

    /*!
     *	@brief Sets the value stored in a byte-node found by @param name.
     *	@ingroup tree-setters
     */
    void bmt_SetByte(BMT_node tree, const BMT_char* name, BMT_byte value);

    /*!
     *	@brief Sets the value stored in a short-node found by @param name.
     *	@ingroup tree-setters
     */
    void bmt_SetShort(BMT_node tree, const BMT_char* name, BMT_short value);

    /*!
     *	@brief Sets the value stored in an int-node found by @param name.
     *	@ingroup tree-setters
     */
    void bmt_SetInt(BMT_node tree, const BMT_char* name, BMT_int value);

    /*!
     *	@brief Sets the value stored in a long-node found by @param name.
     *	@ingroup tree-setters
     */
    void bmt_SetLong(BMT_node tree, const BMT_char* name, BMT_long value);

    /*!
     *	@brief Sets the value stored in a float-node found by @param name.
     *	@ingroup tree-setters
     */
    void bmt_SetFloat(BMT_node tree, const BMT_char* name, BMT_float value);

    /*!
     *	@brief Sets the value stored in a double-node found by @param name.
     *	@ingroup tree-setters
     */
    void bmt_SetDouble(BMT_node tree, const BMT_char* name, BMT_double value);

    /*!
     *	@brief Sets the value stored in a long-node found by @param name.
     *
     *	Issues @ref BMT_STATUS_NO_MEMORY, if failing to copy from @param value.
     *
     *	@ingroup tree-setters
     */
    void bmt_SetString(BMT_node tree, const BMT_char* name, const BMT_char* value);

    /*!
     *	@brief Removes a node from a tree.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param tree is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param tree is not NULL and not a tree.
     *
     *	@ingroup tree-removal
     */
    BMT_bool bmt_Remove(BMT_node* tree, const BMT_char* name);

    /*!
     *	@brief Checks if @param rbt conforms the properties
     *	of a red-black-tree.
     *
     *	@return True, if @param rbt conforms the properties
     *	of a red-black-tree.
     *
     *	@ingroup tree-validation
     */

    BMT_bool bmt_IsValidRBT(const BMT_node rbt);

    /*!
     *	@brief Tells whether or not @param list is a list-node.
     *	A list is defined to be red-coded without a parent.
     *
     *	@return True, if @param node is a list-node.
     *
     *	@ingroup list-validation
     */
    BMT_bool bmt_IsList(const BMT_node list);

    /*!
     *	@brief Returns the length of @param list.
     *
     *	@return The amount of elements contained in @param list,
     *	otherwise 0, if @param list is NULL or not a list-node.
     *
     *	@ingroup list-insertion
     */
    BMT_size bmt_Length(const BMT_node list);

    /*!
     *	@brief Appends a tree-or list-node to the given list.
     *	If @param list points to NULL, a new list will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param list is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_LIST, if the object pointed
     *	to by @param list is not NULL and not a list.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param value
     *	is neither a tree-nor a list-node or NULL.
     *
     *	@ingroup list-insertion
     */
    void bmt_Append(BMT_node* list, const BMT_node value);

    /*!
     *	@brief Appends a byte-node to the given list.
     *	If @param list points to NULL, a new list will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param list is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param list is not NULL and not a list.
     *
     *	@ingroup list-insertion
     */
    void bmt_AppendByte(BMT_node* list, BMT_byte value);

    /*!
     *	@brief Appends a short-node to the given list.
     *	If @param list points to NULL, a new list will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param list is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param list is not NULL and not a list.
     *
     *	@ingroup list-insertion
     */
    void bmt_AppendShort(BMT_node* list, BMT_short value);

    /*!
     *	@brief Appends an int-node to the given list.
     *	If @param list points to NULL, a new list will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param list is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param list is not NULL and not a list.
     *
     *	@ingroup list-insertion
     */
    void bmt_AppendInt(BMT_node* list, BMT_int value);

    /*!
     *	@brief Appends a long-node to the given list.
     *	If @param list points to NULL, a new list will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param list is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param list is not NULL and not a list.
     *
     *	@ingroup list-insertion
     */
    void bmt_AppendLong(BMT_node* list, BMT_long value);

    /*!
     *	@brief Appends a float-node to the given list.
     *	If @param list points to NULL, a new list will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param list is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param list is not NULL and not a list.
     *
     *	@ingroup list-insertion
     */
    void bmt_AppendFloat(BMT_node* list, BMT_float value);

    /*!
     *	@brief Appends a double-node to the given list.
     *	If @param list points to NULL, a new list will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param list is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param list is not NULL and not a list.
     *
     *	@ingroup list-insertion
     */
    void bmt_AppendDouble(BMT_node* list, BMT_double value);

    /*!
     *	@brief Appends a string-node to the given list.
     *	If @param list points to NULL, a new list will be created.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param list is NULL.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param value is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if the object pointed
     *	to by @param list is not NULL and not a list.
     *
     *	@ingroup list-insertion
     */
    void bmt_AppendString(BMT_node* list, const BMT_char* value);

    /*!
     *	@brief Appends a byte-list-node to the given list.
     *	If @param list points to NULL, a new list will be created.
     *	A linked list will be implicitly created from @param values.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param list is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_LIST, if the object pointed
     *	to by @param list is not NULL and not a list.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param value is NULL,
     *	or @param length is less-equal to zero.
     *
     *	@ingroup list-insertion
     */
    void bmt_AppendByteList(BMT_node* list, BMT_size length, const BMT_byte* values);

    /*!
     *	@brief Appends a short-list-node to the given list.
     *	If @param list points to NULL, a new list will be created.
     *	A linked list will be implicitly created from @param values.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param list is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_LIST, if the object pointed
     *	to by @param list is not NULL and not a list.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param value is NULL,
     *	or @param length is less-equal to zero.
     *
     *	@ingroup list-insertion
     */
    void bmt_AppendShortList(BMT_node* list, BMT_size length, const BMT_short* values);

    /*!
     *	@brief Appends an int-list-node to the given list.
     *	If @param list points to NULL, a new list will be created.
     *	A linked list will be implicitly created from @param values.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param list is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_LIST, if the object pointed
     *	to by @param list is not NULL and not a list.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param value is NULL,
     *	or @param length is less-equal to zero.
     *
     *	@ingroup list-insertion
     */
    void bmt_AppendIntList(BMT_node* list, BMT_size length, const BMT_int* values);

    /*!
     *	@brief Appends a long-list-node to the given list.
     *	If @param list points to NULL, a new list will be created.
     *	A linked list will be implicitly created from @param values.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param list is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_LIST, if the object pointed
     *	to by @param list is not NULL and not a list.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param value is NULL,
     *	or @param length is less-equal to zero.
     *
     *	@ingroup list-insertion
     */
    void bmt_AppendLongList(BMT_node* list, BMT_size length, const BMT_long* values);

    /*!
     *	@brief Appends a float-list-node to the given list.
     *	If @param list points to NULL, a new list will be created.
     *	A linked list will be implicitly created from @param values.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param list is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_LIST, if the object pointed
     *	to by @param list is not NULL and not a list.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param value is NULL,
     *	or @param length is less-equal to zero.
     *
     *	@ingroup list-insertion
     */
    void bmt_AppendFloatList(BMT_node* list, BMT_size length, const BMT_float* values);

    /*!
     *	@brief Appends a double-list-node to the given list.
     *	If @param list points to NULL, a new list will be created.
     *	A linked list will be implicitly created from @param values.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param list is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_LIST, if the object pointed
     *	to by @param list is not NULL and not a list.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param value is NULL,
     *	or @param length is less-equal to zero.
     *
     *	@ingroup list-insertion
     */
    void bmt_AppendDoubleList(BMT_node* list, BMT_size length, const BMT_double* values);

    /*!
     *	@brief Appends a string-list-node to the given list.
     *	If @param list points to NULL, a new list will be created.
     *	A linked list will be implicitly created from @param values.
     *	Note that NULL-strings will be ignored.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param list is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_LIST, if the object pointed
     *	to by @param list is not NULL and not a list.
     *
     *	Issues @ref BMT_STATUS_BAD_NAME, if @param name is NULL,
     *	empty or already taken.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param value is NULL,
     *	or @param length is less-equal to zero.
     *
     *	@ingroup list-insertion
     */
    void bmt_AppendStringList(BMT_node* list, BMT_size length, const BMT_char** values);

    /*!
     *	@brief Converts a contiguous array in memory to a linked list.
     *
     *	@return NULL, if @param tag was BMT_TAG_NULL, BMT_TAG_ROOT or BMT_TAG_LIST,
     *	otherwise the newly created list.
     *
     *	@ingroup list-conversion
     */
    BMT_node bmt_ToList(BMT_tag tag, BMT_size length, const void* data);

    /*!
     *	@brief Removes an element of a list at the index @param pos.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param list is NULL.
     *
     *	Issues @ref BMT_STATUS_NOT_A_LIST, if the object pointed
     *	to by @param list is not NULL and not a list.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, @param length is less zero
     *	or exceeds the length of @param list.
     *
     *	@ingroup list-removal
     */
    void bmt_RemoveAt(BMT_node* list, BMT_index pos);

    /*!
     *	@brief Dumps a tree to binary.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if @param is not a tree.
     *
     *	Issues @ref BMT_STATUS_WRITE_ERROR at compression failure.
     *
     *	@return The newly created byte-array.
     *
     *	@ingroup tree-serialization
     */
    BMT_buffer bmt_EncodeTree(const BMT_node tree);

    /*!
     *	@brief Reads a tree-node from a buffer in memory.
     *
     *	Issues @ref BMT_STATUS_NOT_A_TREE, if @param is not a tree.
     *
     *	Issues @ref BMT_STATUS_WRITE_ERROR at decompression failure.
     *
     *	@return The deserialized tree.
     *
     *	@ingroup tree-deserialization
     */
    BMT_node bmt_DecodeTree(BMT_buffer buffer);

    /*!
     *	@brief Deletes an object node.
     *
     *	Issues @ref BMT_STATUS_BAD_VALUE, if @param object
     *	is neither a tree-nor a list-node.
     *
     *	@ingroup deletion
     */
    void bmt_Delete(BMT_node* object);

    /*!
     *	@brief Returns the last error and resets to BMT_STATUS_OK.
     *
     *	@return The last error-code.
     *
     *	@ingroup error-signaling
     */
    BMT_status bmt_GetLastError();

    /*!
     *	@brief Returns an informative error-string by @param status.
     *
     *	@return The error-string
     *
     *	@ingroup error-signaling
     */
    const BMT_char* bmt_GetErrorInfo(BMT_status status);
#ifdef __cplusplus
}
#endif
#endif
