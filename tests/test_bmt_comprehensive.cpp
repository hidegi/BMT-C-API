/**
 * Comprehensive BMT (Binary Merge Tree) test suite
 * Adapted from original mt3.h tests for bmt.h with Google Test
 */
#include "bmt.h"
#include <gtest/gtest.h>
#include <iostream>
#include <random>
#include <string>

// Data set arrangement.
static const BMT_byte byte_data_set_01[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static const BMT_byte byte_data_set_02[] = {-31, -2, -74, -20, 15, -104, -67, 22, 36, -65, 118, -112, -22, -79, -7, -110, 59, -95, -70, 116};
static const BMT_byte byte_data_set_03[] = {-17, 39, 97, -82, -54, -109, 95, -33, -91, 125, -31, -97, 126, -70, -55, 18, 60, -111, 54, 71};
static const BMT_byte byte_data_set_04[] = {-91, 85, -124, -108, 126, 14, 125, 46, -64, 105, -70, 95, -31, -102, 91, 94, -126, -3, -10, 83};
static const BMT_byte byte_data_set_05[] = {-71, 69, -100, -97, -44, -54, -98, -105, 15, 93, -94, 19, 1, 109, 6, 85, 103, 104, -79, 11};
static const BMT_byte byte_data_set_06[] = {-22, -86, 63, 101, 61, -67, 51, 106, 35, -113, 100, 13, 52, -61, -116, 27, 97, 121, 89, -44};
static const BMT_byte byte_data_set_07[] = {-111, -39, -98, 89, -44, -87, 40, 10, 11, -63, -27, 120, -120, 87, 81, -18, 116, 5, -123, 57};
static const BMT_byte byte_data_set_08[] = {82, 70, -20, -14, 93, 69, -87, 85, -32, 11, 87, 81, -120, -2, -81, 115, -85, -65, 61, -78};
static const BMT_byte byte_data_set_09[] = {77, -7, -120, -18, -57, 85, 82, 34, 19, -86, 32, -39, -31, -54, -119, -68, 63, 45, -61, 55};
static const BMT_byte byte_data_set_10[] = {23, -60, -72, 104, -44, 105, 19, -55, 74, 3, 49, 70, -40, -122, 68, -80, 26, 90, -50, -35};
static const BMT_byte* byte_data_set_list[] =
    {
        byte_data_set_01,
        byte_data_set_02,
        byte_data_set_03,
        byte_data_set_04,
        byte_data_set_05,
        byte_data_set_06,
        byte_data_set_07,
        byte_data_set_08,
        byte_data_set_09,
        byte_data_set_10};

static const BMT_short short_data_set_01[] = {-9970, -4214, 17959, 20047, 32484, 10290, 32446, 19327, -6423, -3255, 1920, 20003, 24628, 23842, 1864, -7852, -22168, 18088, -5971, 4675};

static const BMT_int int_data_set_01[] = {113509569, 212226681, 6774860, 87918185, 176696266, 97158013, 194371286, 65171540, 206851927, 87548538, 135515965, 117572157, 129648579, 33338548, 168840390, 153491841, 152125323, 178555076, 203763064, 133882281};

static const BMT_long long_data_set_01[] = {-53223493, -71864115, -16411365, 87570001, -3120436, 80489591, 29512253, 36060369, -11980516, -6666324, -41387253, 42097825, 64386482, -49058131, 57110268, -23297527, 82141980, 77956131, 14495604, 49816788};

static BMT_float float_data_set_01[] = {2.347e+09, -9.876e+09, 5.250e+08, 3.147e+09, -4.333e+08, -7.123e+09, 8.998e+08, 1.074e+10, -2.999e+09, 0.0};

static BMT_double double_data_set_01[] = {1.23456789e9, -2.34567890e8, 3.45678901e9, -4.56789012e7, 5.67890123e9, -6.78901234e8, 7.89012345e9, -8.90123456e6, 9.01234567e9, -1.12345678e10};

static const BMT_char* string_data_set_01[] = {
    "Apple",
    "Banana",
    "Cherry",
    "Date",
    "Elderberry",
    "Fig",
    "Grape",
    "Honeydew",
    "Indian Fig",
    "Jackfruit"};

static const BMT_char* string_data_set_05[] = {
    "Ostrich",
    "Penguin",
    "Quokka",
    "Raccoon",
    "Squirrel",
    "Turtle",
    NULL,
    NULL,
    "Uakari",
    "Vulture",
    NULL,
    "Wolf",
    "Xerus"};

static const BMT_char** string_data_set_list[] =
    {
        string_data_set_01,
        string_data_set_01,
        string_data_set_01,
        string_data_set_01,
        string_data_set_05,
        string_data_set_01,
        string_data_set_01,
        string_data_set_01,
        string_data_set_01,
        string_data_set_01};

class BMTFixture : public testing::Test
{
    protected:
        void SetUp() override
        {
            tree = bmt_AllocTree();
        }

        void TearDown() override
        {
            bmt_Delete(&tree);
        }

        BMT_node tree;
};

static BMT_node createMock()
{
    BMT_node subtree = NULL;
    bmt_InsertString(&subtree, "str1", "motex");
    bmt_InsertString(&subtree, "str2", "gaming");
    bmt_InsertString(&subtree, "str3", "is");
    bmt_InsertString(&subtree, "str4", "ugly");

    BMT_node list = NULL;
    bmt_Append(&list, subtree);
    bmt_Append(&list, subtree);
    bmt_Append(&list, subtree);
    bmt_Append(&list, subtree);

    BMT_node tree = NULL;

    bmt_InsertByte(&tree, "byte_1", 1);
    bmt_InsertShort(&tree, "short_1", -123);
    bmt_InsertInt(&tree, "int_1", 1234567);
    bmt_InsertLong(&tree, "long_1", 1234567485);
    bmt_InsertFloat(&tree, "float_1", 134.45f);
    bmt_InsertDouble(&tree, "double_1", 5423);
    bmt_InsertString(&tree, "string_1", "motex");
    bmt_InsertByte(&tree, "byte_2", 1);
    bmt_InsertShort(&tree, "short_2", -123);
    bmt_InsertInt(&tree, "int_2", 1234567);
    bmt_InsertLong(&tree, "long_2", 1234567485);
    bmt_InsertFloat(&tree, "float_2", 134.45f);
    bmt_InsertDouble(&tree, "double_2", 5423);
    bmt_InsertString(&tree, "string_2", "gaming");
    bmt_Insert(&tree, "subtree", subtree);
    bmt_Insert(&tree, "list", list);

    BMT_node multi_list = NULL;
    bmt_Append(&multi_list, list);
    bmt_Append(&multi_list, list);
    bmt_Append(&multi_list, list);

    BMT_node multi_multi_list = NULL;
    bmt_Append(&multi_multi_list, multi_list);
    bmt_Append(&multi_multi_list, multi_list);
    bmt_Append(&multi_multi_list, multi_list);
    bmt_Append(&multi_multi_list, multi_list);
    bmt_Append(&multi_multi_list, multi_list);

    BMT_node multi_multi_multi_list = NULL;
    bmt_Append(&multi_multi_multi_list, multi_multi_list);

    BMT_byte byte_array[] = {1, 2, 3, 4, 5, -6, -7, -8, -9};
    BMT_short short_array[] = {1, 2, 3, 4, 5, -6, -7, -8, -9};
    BMT_int int_array[] = {1, 2, 3, 4, 5, -6, -7, -8, -9};
    BMT_long long_array[] = {1, 2, 3, 4, 5, -6, -7, -8, -9};
    BMT_float float_array[] = {1.3f, 2.4f, 5.4f, 252.f, 19.f, 43.f, 74.f};
    BMT_double double_array[] = {1.3, 2.4, 5.4, 252.0, 19.0, 43.0, 74.0};
    const BMT_char* string_array[] = {"hda1666", "sp1667", "fjiaw", "betel"};

    BMT_node byte_list = NULL;
    bmt_AppendByteList(&byte_list, sizeof(byte_array) / sizeof(BMT_byte), byte_array);
    bmt_AppendByteList(&byte_list, sizeof(byte_array) / sizeof(BMT_byte), byte_array);
    bmt_AppendByteList(&byte_list, sizeof(byte_array) / sizeof(BMT_byte), byte_array);
    bmt_AppendByteList(&byte_list, sizeof(byte_array) / sizeof(BMT_byte), byte_array);

    bmt_InsertByteList(&tree, "byte_array", 9, byte_array);
    bmt_InsertShortList(&tree, "short_aray", 9, short_array);
    bmt_InsertIntList(&tree, "int_aray", 9, int_array);
    bmt_InsertLongList(&tree, "long_aray", 9, long_array);
    bmt_InsertFloatList(&tree, "float_aray", sizeof(float_array) / sizeof(BMT_float), float_array);
    bmt_InsertDoubleList(&tree, "double_aray", sizeof(double_array) / sizeof(BMT_double), double_array);
    bmt_InsertStringList(&tree, "string_array", sizeof(string_array) / sizeof(BMT_char*), string_array);
    bmt_Insert(&tree, "multi_list", multi_list);
    bmt_Insert(&tree, "byte_list", byte_list);
    bmt_Insert(&tree, "multi_multi_list", multi_multi_list);
    bmt_Insert(&tree, "multi_multi_multi_list", multi_multi_multi_list);

    bmt_Delete(&subtree);
    bmt_Delete(&list);
    bmt_Delete(&multi_list);
    bmt_Delete(&multi_multi_list);
    bmt_Delete(&multi_multi_multi_list);
    return tree;
}

// RBT Validation Tests
TEST(BMTValidation, ValidRBT_Simple)
{
    _BMT_node n1, n2, n3;

    n1.parent = NULL;
    n1.minor = &n2;
    n1.major = &n3;
    n2.parent = n3.parent = &n1;

    n1.red = false;
    n2.red = false;
    n3.red = false;

    n2.major = n2.minor = NULL;
    n3.major = n3.minor = NULL;
    ASSERT_TRUE(bmt_IsValidRBT(&n1));
}

TEST(BMTValidation, InvalidRBT_RedViolation)
{
    _BMT_node n1, n2, n3, n4, n5;

    n1.parent = NULL;
    n1.minor = &n2;
    n1.major = &n3;
    n2.parent = n3.parent = &n1;

    n1.red = false;
    n2.red = false;
    n3.red = true; // Red parent

    n2.major = n2.minor = NULL;
    n3.minor = &n4;
    n3.major = &n5;

    n4.parent = n5.parent = &n3;
    n4.major = n4.minor = NULL;
    n5.major = n5.minor = NULL;
    n4.red = n5.red = true; // Red children - violation!

    ASSERT_FALSE(bmt_IsValidRBT(&n1));
}

TEST(BMTValidation, ValidRBT_Complex)
{
    _BMT_node n1, n2, n3, n4, n5;

    n1.parent = NULL;
    n1.minor = &n2;
    n1.major = &n3;
    n2.parent = n3.parent = &n1;

    n1.red = false;
    n2.red = false;
    n3.red = false;

    n2.major = n2.minor = NULL;
    n3.minor = &n4;
    n3.major = &n5;

    n4.parent = n5.parent = &n3;
    n4.major = n4.minor = NULL;
    n5.major = n5.minor = NULL;
    n4.red = n5.red = true;

    ASSERT_TRUE(bmt_IsValidRBT(&n1));
}

TEST(BMTValidation, InvalidRBT_BlackHeightViolation)
{
    _BMT_node n1, n2, n3, n4, n5;

    n1.parent = NULL;
    n1.minor = &n2;
    n1.major = &n3;
    n2.parent = n3.parent = &n1;

    n1.red = false;
    n2.red = false;
    n3.red = false;

    n2.major = n2.minor = NULL;
    n3.minor = &n4;
    n3.major = &n5;

    n4.parent = n5.parent = &n3;
    n4.major = n4.minor = NULL;
    n5.major = n5.minor = NULL;
    n4.red = n5.red = false; // Black height violation

    ASSERT_FALSE(bmt_IsValidRBT(&n1));
}

// Equality Tests
TEST(BMTEquality, SimpleTree)
{
    BMT_node treeA = NULL, treeB = NULL;

    bmt_InsertByte(&treeA, "byte1", 1);
    bmt_InsertByte(&treeA, "byte2", 2);
    bmt_InsertByte(&treeA, "byte3", 3);
    bmt_InsertByte(&treeA, "byte4", 4);
    bmt_InsertByte(&treeA, "byte5", 5);
    bmt_InsertByte(&treeA, "byte6", 6);

    bmt_InsertByte(&treeB, "byte1", 1);
    bmt_InsertByte(&treeB, "byte2", 2);
    bmt_InsertByte(&treeB, "byte3", 3);
    bmt_InsertByte(&treeB, "byte4", 4);
    bmt_InsertByte(&treeB, "byte5", 5);
    bmt_InsertByte(&treeB, "byte6", 6);

    EXPECT_TRUE(bmt_IsEqual(treeA, treeB));
    bmt_Delete(&treeA);
    bmt_Delete(&treeB);
}

TEST(BMTEquality, ComplexTree)
{
    BMT_node tree1 = createMock();
    BMT_node tree2 = createMock();
    ASSERT_TRUE(tree1 && tree2);
    EXPECT_TRUE(bmt_IsEqual(tree1, tree2));
    bmt_Delete(&tree1);
    bmt_Delete(&tree2);
}

// Insertion Tests
TEST_F(BMTFixture, ByteInsertion)
{
    for (BMT_size i = 0; i < 10; i++)
    {
        bmt_InsertByte(&tree, std::string("byte_" + std::to_string(i + 1)).c_str(), byte_data_set_01[i]);
    }
    ASSERT_TRUE(bmt_IsValidRBT(tree));
}

TEST_F(BMTFixture, ShortInsertion)
{
    for (BMT_size i = 0; i < 20; i++)
    {
        bmt_InsertShort(&tree, std::string("short_" + std::to_string(i + 1)).c_str(), short_data_set_01[i]);
    }
    ASSERT_TRUE(bmt_IsValidRBT(tree));
}

TEST_F(BMTFixture, IntInsertion)
{
    for (BMT_size i = 0; i < 20; i++)
    {
        bmt_InsertInt(&tree, std::string("int_" + std::to_string(i + 1)).c_str(), int_data_set_01[i]);
    }
    ASSERT_TRUE(bmt_IsValidRBT(tree));
}

TEST_F(BMTFixture, LongInsertion)
{
    for (BMT_size i = 0; i < 20; i++)
    {
        bmt_InsertLong(&tree, std::string("long_" + std::to_string(i + 1)).c_str(), long_data_set_01[i]);
    }
    ASSERT_TRUE(bmt_IsValidRBT(tree));
}

TEST_F(BMTFixture, FloatInsertion)
{
    for (BMT_size i = 0; i < 10; i++)
    {
        bmt_InsertFloat(&tree, std::string("float_" + std::to_string(i + 1)).c_str(), float_data_set_01[i]);
    }
    ASSERT_TRUE(bmt_IsValidRBT(tree));
}

TEST_F(BMTFixture, DoubleInsertion)
{
    for (BMT_size i = 0; i < 10; i++)
    {
        bmt_InsertDouble(&tree, std::string("double_" + std::to_string(i + 1)).c_str(), double_data_set_01[i]);
    }
    ASSERT_TRUE(bmt_IsValidRBT(tree));
}

TEST_F(BMTFixture, StringInsertion)
{
    for (BMT_size i = 0; i < 10; i++)
    {
        bmt_InsertString(&tree, std::string("string_" + std::to_string(i + 1)).c_str(), string_data_set_01[i]);
    }
    ASSERT_TRUE(bmt_IsValidRBT(tree));
}

TEST_F(BMTFixture, TreeInsertion)
{
    for (BMT_size i = 0; i < 10; i++)
    {
        BMT_node subtree = NULL;
        bmt_InsertByte(&subtree, "byte", byte_data_set_01[i]);
        bmt_InsertShort(&subtree, "short", short_data_set_01[i]);
        bmt_InsertInt(&subtree, "int", int_data_set_01[i]);
        bmt_InsertLong(&subtree, "long", long_data_set_01[i]);
        bmt_InsertFloat(&subtree, "float", float_data_set_01[i]);
        bmt_InsertDouble(&subtree, "double", double_data_set_01[i]);
        bmt_InsertString(&subtree, "string", string_data_set_01[i]);
        bmt_Insert(&tree, std::string("tree_" + std::to_string(i + 1)).c_str(), subtree);
        bmt_Delete(&subtree);
    }
    ASSERT_TRUE(bmt_IsValidRBT(tree));
}

// List Array Insertion Tests
TEST_F(BMTFixture, ByteListInsert)
{
    for (int i = 0; i < 10; i++)
    {
        std::string name = "byte_array_" + std::to_string(i + 1);
        bmt_InsertByteList(&tree, name.c_str(), 10, byte_data_set_list[i]);
    }
    ASSERT_TRUE(bmt_IsValidRBT(tree));
}

TEST_F(BMTFixture, StringListInsert)
{
    for (int i = 0; i < 10; i++)
    {
        std::string name = "string_array_" + std::to_string(i + 1);
        bmt_InsertStringList(&tree, name.c_str(), 10, string_data_set_list[i]);
    }
    ASSERT_TRUE(bmt_IsValidRBT(tree));
}

TEST_F(BMTFixture, ByteMultiListInsert)
{
    BMT_node list = NULL;

    for (BMT_size i = 0; i < 10; i++)
    {
        bmt_AppendByteList(&list, 10, byte_data_set_list[i]);
    }

    bmt_Insert(&tree, "list", list);
    EXPECT_TRUE(bmt_IsValidRBT(tree));
    bmt_Delete(&list);
}

TEST_F(BMTFixture, StringMultiListInsert)
{
    BMT_node list = NULL;

    for (BMT_size i = 0; i < 10; i++)
    {
        bmt_AppendStringList(&list, 10, string_data_set_list[i]);
    }

    bmt_Insert(&tree, "list", list);
    EXPECT_TRUE(bmt_IsValidRBT(tree));
    bmt_Delete(&list);
}

// Deletion Tests
TEST_F(BMTFixture, SimpleDeletion)
{
    for (BMT_size i = 0; i < 10; i++)
    {
        bmt_InsertByte(&tree, std::string("byte_" + std::to_string(i + 1)).c_str(), byte_data_set_01[i]);
    }

    ASSERT_TRUE(bmt_IsValidRBT(tree));
    ASSERT_TRUE(bmt_Remove(&tree, "byte_3"));
    ASSERT_TRUE(bmt_Remove(&tree, "byte_2"));
    ASSERT_TRUE(bmt_Remove(&tree, "byte_1"));
    ASSERT_TRUE(bmt_Remove(&tree, "byte_6"));
    ASSERT_TRUE(bmt_Remove(&tree, "byte_5"));
    ASSERT_TRUE(bmt_Remove(&tree, "byte_4"));
    ASSERT_TRUE(bmt_Remove(&tree, "byte_8"));
    ASSERT_TRUE(bmt_IsValidRBT(tree));
}

bool containsNumber(int n, const int* array, int length)
{
    for (int i = 0; i < length; i++)
        if (array[i] == n)
            return true;
    return false;
}

#define LENGTH 100

TEST_F(BMTFixture, RandomDeletion)
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dst(1, LENGTH);
    int* array = new int[LENGTH];
    for (int i = 0; i < LENGTH; i++)
    {
        int num = dst(rng);
        while (containsNumber(num, array, LENGTH))
            num = dst(rng);
        array[i] = num;
        bmt_InsertInt(&tree, std::string("int_" + std::to_string(array[i])).c_str(), array[i]);
    }

    delete[] array;
    ASSERT_TRUE(bmt_IsValidRBT(tree));
    int indices[100] =
        {
            49, 14, 48, 9, 35, 27, 64, 58, 18, 55, 46, 20, 63, 11, 4, 70, 32, 90, 41, 66, 16, 76, 77, 33, 67, 43, 29, 57, 81, 6, 95, 26, 71, 60, 56, 78, 93, 74, 98, 75, 72, 23, 7, 47, 5, 17, 82, 100, 22, 52, 91, 36, 94, 2, 30, 34, 25, 21, 19, 42, 50, 80, 96, 59, 53, 86, 12, 87, 69, 61, 92, 83, 28, 68, 10, 54, 31, 8, 37, 85, 84, 3, 38, 24, 40, 45, 99, 88, 51, 44, 1, 15, 13, 89, 65, 39, 79, 73, 97, 62};
    for (int i = 0; i < LENGTH - 1; i++)
    {
        EXPECT_TRUE(bmt_Remove(&tree, std::string("int_" + std::to_string(indices[i])).c_str()));
        ASSERT_TRUE(bmt_IsValidRBT(tree));
    }

    ASSERT_TRUE(bmt_IsValidRBT(tree));
}

// Setter/Getter Tests
TEST_F(BMTFixture, SetAndGetValues)
{
    BMT_node tree = NULL;
    bmt_InsertByte(&tree, "byte_1", 1);
    bmt_SetByte(tree, "byte_1", 17);
    ASSERT_EQ(17, bmt_Get(tree, "byte_1")->payload.tag_byte);

    bmt_InsertShort(&tree, "short_1", 1);
    bmt_SetShort(tree, "short_1", 1667);
    ASSERT_EQ(1667, bmt_Get(tree, "short_1")->payload.tag_short);

    bmt_InsertInt(&tree, "int_1", 1);
    bmt_SetInt(tree, "int_1", 1667);
    ASSERT_EQ(1667, bmt_Get(tree, "int_1")->payload.tag_int);

    bmt_InsertLong(&tree, "long_1", 1);
    bmt_SetLong(tree, "long_1", 1667);
    ASSERT_EQ(1667, bmt_Get(tree, "long_1")->payload.tag_long);

    bmt_InsertFloat(&tree, "float_1", 143.f);
    bmt_SetFloat(tree, "float_1", 135.f);
    ASSERT_NEAR(135.f, bmt_Get(tree, "float_1")->payload.tag_float, 0.001f);

    bmt_InsertDouble(&tree, "double_1", 344954.9485);
    bmt_SetDouble(tree, "double_1", 135.0);
    ASSERT_NEAR(135.0, bmt_Get(tree, "double_1")->payload.tag_double, 0.0001);

    bmt_InsertString(&tree, "string_1", "motex gaming 1667");
    bmt_SetString(tree, "string_1", "fjiaw");
    ASSERT_STREQ("fjiaw", bmt_Get(tree, "string_1")->payload.tag_string);
    ASSERT_TRUE(bmt_IsValidRBT(tree));
    bmt_Delete(&tree);
}

TEST_F(BMTFixture, SetValuesInComplexTree)
{
    bmt_Delete(&tree);
    tree = createMock();

    bmt_SetString(tree, "string_2", "haha");
    ASSERT_STREQ("haha", bmt_Get(tree, "string_2")->payload.tag_string);

    bmt_SetByte(tree, "byte_1", 2);
    ASSERT_EQ(2, bmt_Get(tree, "byte_1")->payload.tag_byte);

    bmt_SetShort(tree, "short_1", 3);
    ASSERT_EQ(3, bmt_Get(tree, "short_1")->payload.tag_short);

    bmt_SetInt(tree, "int_1", 4);
    ASSERT_EQ(4, bmt_Get(tree, "int_1")->payload.tag_int);

    bmt_SetLong(tree, "long_1", 5);
    ASSERT_EQ(5, bmt_Get(tree, "long_1")->payload.tag_long);

    bmt_SetFloat(tree, "float_1", 123.456f);
    ASSERT_NEAR(123.456f, bmt_Get(tree, "float_1")->payload.tag_float, 0.001f);

    bmt_SetDouble(tree, "double_2", 3.141592654);
    ASSERT_NEAR(3.141592654, bmt_Get(tree, "double_2")->payload.tag_double, 0.0001);

    BMT_node multi_list = bmt_Get(tree, "multi_list");
    ASSERT_TRUE(multi_list && multi_list->payload.tag_object != NULL);

    BMT_node multi_multi_list = bmt_Get(tree, "multi_multi_list");
    ASSERT_TRUE(multi_multi_list && multi_multi_list->payload.tag_object != NULL);

    BMT_node multi_multi_multi_list = bmt_Get(tree, "multi_multi_multi_list");
    ASSERT_TRUE(multi_multi_multi_list && multi_multi_multi_list->payload.tag_object != NULL);
}

// List Element Removal Test
TEST_F(BMTFixture, ListElementRemoval)
{
    BMT_node list = bmt_AllocList();
    bmt_AppendByte(&list, 1);
    bmt_AppendByte(&list, 2);
    bmt_AppendByte(&list, 3);
    bmt_AppendByte(&list, 4);
    bmt_AppendByte(&list, 5);
    bmt_AppendByte(&list, 6);
    bmt_AppendByte(&list, 7);
    bmt_AppendInt(&list, 1667);
    bmt_AppendString(&list, "hello");
    bmt_RemoveAt(&list, 0);

    for (BMT_node cursor = list; cursor != NULL; cursor = cursor->major)
    {
        EXPECT_NE(1, cursor->payload.tag_byte);
    }

    bmt_Delete(&list);
}

// Serialization Test
TEST(BMTSerialization, EncodeAndDecode)
{
    BMT_node tree1 = createMock();
    BMT_buffer buffer = bmt_EncodeTree(tree1);
    BMT_node tree2 = bmt_DecodeTree(buffer);
    EXPECT_TRUE(bmt_IsEqual(tree1, tree2));
    bmt_Delete(&tree1);
    bmt_Delete(&tree2);
    bmt_BufferFree(&buffer);
}
