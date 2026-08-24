#-----------------------------------------------------------------------------
# BMT Test Suite Configuration
# Tests for the BMT C-API
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Google Test Setup
#-----------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
    GIT_SHALLOW    TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

#-----------------------------------------------------------------------------
# Test Suite — BMT C-API
#-----------------------------------------------------------------------------

# Basic allocation and insertion tests (custom framework, own main)
bmt_add_test(test_bmt_basic
    SOURCES test_bmt_basic.cpp
    DEPENDS bmt
)

# Tree structure and RBT validation tests (custom framework, own main)
bmt_add_test(test_bmt_trees
    SOURCES test_bmt_trees.cpp
    DEPENDS bmt
)

# Comprehensive insertion, deletion, and serialization tests (Google Test)
bmt_add_test(test_bmt_comprehensive
    SOURCES test_bmt_comprehensive.cpp
    DEPENDS bmt
    GTEST
)
