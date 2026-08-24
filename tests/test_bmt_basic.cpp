/**
 * Basic BMT functionality tests
 * Tests allocation, deallocation, and basic operations
 */
#include "bmt.h"
#include <cassert>
#include <cstring>
#include <iostream>

// Simple test framework
int g_tests_run = 0;
int g_tests_passed = 0;

#define TEST(name)                                                                                                     \
    void name();                                                                                                       \
    struct name##_runner                                                                                               \
    {                                                                                                                  \
            name##_runner()                                                                                            \
            {                                                                                                          \
                std::cout << "Running " << #name << "... ";                                                            \
                g_tests_run++;                                                                                         \
                try                                                                                                    \
                {                                                                                                      \
                    name();                                                                                            \
                    g_tests_passed++;                                                                                  \
                    std::cout << "PASSED\n";                                                                           \
                }                                                                                                      \
                catch (...)                                                                                            \
                {                                                                                                      \
                    std::cout << "FAILED\n";                                                                           \
                }                                                                                                      \
            }                                                                                                          \
    } name##_instance;                                                                                                 \
    void name()

#define ASSERT(condition)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(condition))                                                                                              \
        {                                                                                                              \
            std::cerr << "  Assertion failed: " << #condition << " at line " << __LINE__ << std::endl;                 \
            throw std::runtime_error("Test failed");                                                                   \
        }                                                                                                              \
    } while (0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NE(a, b) ASSERT((a) != (b))
#define ASSERT_TRUE(x) ASSERT((x))
#define ASSERT_FALSE(x) ASSERT(!(x))

// Test: Allocate and free tree
TEST(test_alloc_tree)
{
    BMT_node tree = bmt_AllocTree();
    ASSERT_NE(tree, nullptr);
    ASSERT_TRUE(bmt_IsTree(tree));
    ASSERT_FALSE(bmt_IsList(tree));
    // AllocTree returns a sentinel, actual root is the payload
    bmt_Delete(&tree);
    ASSERT_EQ(tree, nullptr);
}

// Test: Allocate and free list
TEST(test_alloc_list)
{
    BMT_node list = bmt_AllocList();
    ASSERT_NE(list, nullptr);
    ASSERT_TRUE(bmt_IsList(list));
    ASSERT_FALSE(bmt_IsTree(list));
    // AllocList returns a sentinel, actual root is the payload
    bmt_Delete(&list);
    ASSERT_EQ(list, nullptr);
}

// Test: Insert primitives into tree
TEST(test_insert_primitives)
{
    BMT_node tree = nullptr;

    bmt_InsertByte(&tree, "byte_val", 42);
    ASSERT_EQ(bmt_GetLastError(), BMT_STATUS_OK);
    ASSERT_NE(tree, nullptr);

    bmt_InsertShort(&tree, "short_val", 1234);
    ASSERT_EQ(bmt_GetLastError(), BMT_STATUS_OK);

    bmt_InsertInt(&tree, "int_val", 999999);
    ASSERT_EQ(bmt_GetLastError(), BMT_STATUS_OK);

    bmt_InsertLong(&tree, "long_val", 123456789L);
    ASSERT_EQ(bmt_GetLastError(), BMT_STATUS_OK);

    bmt_InsertFloat(&tree, "float_val", 3.14f);
    ASSERT_EQ(bmt_GetLastError(), BMT_STATUS_OK);

    bmt_InsertDouble(&tree, "double_val", 2.71828);
    ASSERT_EQ(bmt_GetLastError(), BMT_STATUS_OK);

    bmt_InsertString(&tree, "string_val", "Hello, World!");
    ASSERT_EQ(bmt_GetLastError(), BMT_STATUS_OK);

    bmt_Delete(&tree);
}

// Test: Get values from tree
TEST(test_get_values)
{
    BMT_node tree = nullptr;

    bmt_InsertByte(&tree, "test_byte", 100);
    bmt_InsertString(&tree, "test_string", "TestValue");

    BMT_node byte_node = bmt_Get(tree, "test_byte");
    ASSERT_NE(byte_node, nullptr);
    ASSERT_EQ(byte_node->tag, BMT_TAG_BYTE);
    ASSERT_EQ(byte_node->payload.tag_byte, 100);

    BMT_node string_node = bmt_Get(tree, "test_string");
    ASSERT_NE(string_node, nullptr);
    ASSERT_EQ(string_node->tag, BMT_TAG_STRING);
    ASSERT_EQ(std::strcmp(string_node->payload.tag_string, "TestValue"), 0);

    // Test non-existent key
    BMT_node missing = bmt_Get(tree, "nonexistent");
    ASSERT_EQ(missing, nullptr);

    bmt_Delete(&tree);
}

// Test: Error handling
TEST(test_error_handling)
{
    BMT_node tree = nullptr;

    // Duplicate name should fail
    bmt_InsertInt(&tree, "duplicate", 1);
    ASSERT_EQ(bmt_GetLastError(), BMT_STATUS_OK);

    bmt_InsertInt(&tree, "duplicate", 2);
    BMT_status error = bmt_GetLastError();
    ASSERT_EQ(error, BMT_STATUS_BAD_NAME);

    // Empty name should fail
    bmt_InsertInt(&tree, "", 3);
    error = bmt_GetLastError();
    ASSERT_EQ(error, BMT_STATUS_BAD_NAME);

    // NULL name should fail
    bmt_InsertInt(&tree, nullptr, 4);
    error = bmt_GetLastError();
    ASSERT_EQ(error, BMT_STATUS_BAD_NAME);

    bmt_Delete(&tree);
}

// Test: Copy tree
TEST(test_copy_tree)
{
    BMT_node tree = nullptr;

    bmt_InsertInt(&tree, "value1", 100);
    bmt_InsertString(&tree, "value2", "test");

    BMT_node copy = bmt_Copy(tree);
    ASSERT_NE(copy, nullptr);
    ASSERT_TRUE(bmt_IsEqual(tree, copy));

    // Modify copy - original should be unchanged
    bmt_SetInt(copy, "value1", 200);

    BMT_node orig_node = bmt_Get(tree, "value1");
    BMT_node copy_node = bmt_Get(copy, "value1");

    ASSERT_EQ(orig_node->payload.tag_int, 100);
    ASSERT_EQ(copy_node->payload.tag_int, 200);

    bmt_Delete(&tree);
    bmt_Delete(&copy);
}

// Test: Remove from tree
TEST(test_remove)
{
    BMT_node tree = nullptr;

    bmt_InsertInt(&tree, "key1", 1);
    bmt_InsertInt(&tree, "key2", 2);
    bmt_InsertInt(&tree, "key3", 3);

    ASSERT_NE(bmt_Get(tree, "key2"), nullptr);

    BMT_bool removed = bmt_Remove(&tree, "key2");
    ASSERT_TRUE(removed);
    ASSERT_EQ(bmt_Get(tree, "key2"), nullptr);

    // key1 and key3 should still exist
    ASSERT_NE(bmt_Get(tree, "key1"), nullptr);
    ASSERT_NE(bmt_Get(tree, "key3"), nullptr);

    bmt_Delete(&tree);
}

int main()
{
    std::cout << "\n=== BMT Basic Tests ===\n\n";

    // Tests run automatically via static initializers

    std::cout << "\n======================\n";
    std::cout << "Tests run: " << g_tests_run << "\n";
    std::cout << "Tests passed: " << g_tests_passed << "\n";
    std::cout << "Tests failed: " << (g_tests_run - g_tests_passed) << "\n";

    return (g_tests_run == g_tests_passed) ? 0 : 1;
}
