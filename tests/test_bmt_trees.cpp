/**
 * BMT tree structure tests
 * Tests nested trees, RBT validation, and complex tree operations
 */
#include "bmt.h"
#include <cassert>
#include <iostream>

int g_tests_run = 0;
int g_tests_passed = 0;

#define TEST(name)                                          \
    void name();                                            \
    struct name##_runner                                    \
    {                                                       \
            name##_runner()                                 \
            {                                               \
                std::cout << "Running " << #name << "... "; \
                g_tests_run++;                              \
                try                                         \
                {                                           \
                    name();                                 \
                    g_tests_passed++;                       \
                    std::cout << "PASSED\n";                \
                }                                           \
                catch (...)                                 \
                {                                           \
                    std::cout << "FAILED\n";                \
                }                                           \
            }                                               \
    } name##_instance;                                      \
    void name()

#define ASSERT(condition)                                                                              \
    do                                                                                                 \
    {                                                                                                  \
        if (!(condition))                                                                              \
        {                                                                                              \
            std::cerr << "  Assertion failed: " << #condition << " at line " << __LINE__ << std::endl; \
            throw std::runtime_error("Test failed");                                                   \
        }                                                                                              \
    } while (0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NE(a, b) ASSERT((a) != (b))
#define ASSERT_TRUE(x) ASSERT((x))

// Test: Create nested tree structures
TEST(test_nested_trees)
{
    BMT_node root = nullptr;

    // Create parent tree
    bmt_CreateTree(&root, "child_tree");
    ASSERT_EQ(bmt_GetLastError(), BMT_STATUS_OK);
    ASSERT_NE(root, nullptr);

    // Get the child tree node
    BMT_node child = bmt_Get(root, "child_tree");
    ASSERT_NE(child, nullptr);
    ASSERT_EQ(child->tag, BMT_TAG_ROOT);
    // The actual tree is in the payload
    BMT_node child_tree = child->payload.tag_object;

    // Add data to child tree
    bmt_InsertInt(&child_tree, "nested_value", 42);
    ASSERT_EQ(bmt_GetLastError(), BMT_STATUS_OK);

    // Verify nested value
    BMT_node nested = bmt_Get(child_tree, "nested_value");
    ASSERT_NE(nested, nullptr);
    ASSERT_EQ(nested->payload.tag_int, 42);

    bmt_Delete(&root);
}

// Test: RBT validation after many insertions
TEST(test_rbt_validation)
{
    BMT_node tree = nullptr;

    // Insert many values to trigger rotations
    for (int i = 0; i < 100; i++)
    {
        char name[32];
        snprintf(name, sizeof(name), "key_%d", i);
        bmt_InsertInt(&tree, name, i);
        ASSERT_EQ(bmt_GetLastError(), BMT_STATUS_OK);
    }

    // Verify tree is still valid RBT
    ASSERT_TRUE(bmt_IsValidRBT(tree));

    bmt_Delete(&tree);
}

// Test: Multiple levels of nesting
TEST(test_deep_nesting)
{
    BMT_node root = nullptr;

    bmt_CreateTree(&root, "level1");
    BMT_node level1 = bmt_Get(root, "level1")->payload.tag_object;

    bmt_CreateTree(&level1, "level2");
    BMT_node level2 = bmt_Get(level1, "level2")->payload.tag_object;

    bmt_CreateTree(&level2, "level3");
    BMT_node level3 = bmt_Get(level2, "level3")->payload.tag_object;

    bmt_InsertString(&level3, "deep_value", "I'm nested!");

    // Verify we can access the deep value
    BMT_node deep = bmt_Get(level3, "deep_value");
    ASSERT_NE(deep, nullptr);
    ASSERT_EQ(deep->tag, BMT_TAG_STRING);

    bmt_Delete(&root);
}

// Test: Tree with mixed types
TEST(test_mixed_types)
{
    BMT_node tree = nullptr;

    bmt_InsertByte(&tree, "a_byte", 1);
    bmt_InsertShort(&tree, "a_short", 2);
    bmt_InsertInt(&tree, "an_int", 3);
    bmt_InsertLong(&tree, "a_long", 4);
    bmt_InsertFloat(&tree, "a_float", 5.5f);
    bmt_InsertDouble(&tree, "a_double", 6.6);
    bmt_InsertString(&tree, "a_string", "seven");
    bmt_CreateTree(&tree, "a_tree");
    bmt_CreateList(&tree, "a_list");

    // Verify all exist with correct types
    ASSERT_EQ(bmt_Get(tree, "a_byte")->tag, BMT_TAG_BYTE);
    ASSERT_EQ(bmt_Get(tree, "a_short")->tag, BMT_TAG_SHORT);
    ASSERT_EQ(bmt_Get(tree, "an_int")->tag, BMT_TAG_INT);
    ASSERT_EQ(bmt_Get(tree, "a_long")->tag, BMT_TAG_LONG);
    ASSERT_EQ(bmt_Get(tree, "a_float")->tag, BMT_TAG_FLOAT);
    ASSERT_EQ(bmt_Get(tree, "a_double")->tag, BMT_TAG_DOUBLE);
    ASSERT_EQ(bmt_Get(tree, "a_string")->tag, BMT_TAG_STRING);
    // CreateTree and CreateList create nested structures
    ASSERT_NE(bmt_Get(tree, "a_tree"), nullptr);
    ASSERT_NE(bmt_Get(tree, "a_list"), nullptr);

    ASSERT_TRUE(bmt_IsValidRBT(tree));

    bmt_Delete(&tree);
}

// Test: Set existing values
TEST(test_set_values)
{
    BMT_node tree = nullptr;

    bmt_InsertInt(&tree, "value", 100);
    BMT_node node = bmt_Get(tree, "value");
    ASSERT_EQ(node->payload.tag_int, 100);

    bmt_SetInt(tree, "value", 200);
    node = bmt_Get(tree, "value");
    ASSERT_EQ(node->payload.tag_int, 200);

    bmt_SetInt(tree, "value", 300);
    node = bmt_Get(tree, "value");
    ASSERT_EQ(node->payload.tag_int, 300);

    bmt_Delete(&tree);
}

// Test: Remove and reinsert
TEST(test_remove_reinsert)
{
    BMT_node tree = nullptr;

    bmt_InsertInt(&tree, "key", 1);
    ASSERT_NE(bmt_Get(tree, "key"), nullptr);

    bmt_Remove(&tree, "key");
    ASSERT_EQ(bmt_Get(tree, "key"), nullptr);

    // Should be able to reinsert with same name
    bmt_InsertInt(&tree, "key", 2);
    ASSERT_EQ(bmt_GetLastError(), BMT_STATUS_OK);

    BMT_node node = bmt_Get(tree, "key");
    ASSERT_NE(node, nullptr);
    ASSERT_EQ(node->payload.tag_int, 2);

    bmt_Delete(&tree);
}

int main()
{
    std::cout << "\n=== BMT Tree Tests ===\n\n";

    std::cout << "\n======================\n";
    std::cout << "Tests run: " << g_tests_run << "\n";
    std::cout << "Tests passed: " << g_tests_passed << "\n";
    std::cout << "Tests failed: " << (g_tests_run - g_tests_passed) << "\n";

    return (g_tests_run == g_tests_passed) ? 0 : 1;
}
