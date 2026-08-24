macro(bmt_set_default_option var default type docstring)
        if(NOT DEFINED ${var})
                set(${var} ${default})
        endif()
        set(${var} ${${var}} CACHE ${type} ${docstring} FORCE)
endmacro()

function(bmt_set_stdlib target)
    # for gcc on Windows, apply the bmt_USE_STATIC_STD_LIBS option if it is enabled
    if(bmt_PLATFORM_WINDOWS)
        if(bmt_COMPILER_GCC)
            if(bmt_USE_STATIC_STD_LIBS AND NOT bmt_COMPILER_GCC_TDM)
                target_link_libraries(${target} PRIVATE "-static-libgcc" "-static-libstdc++")
            elseif(NOT bmt_USE_STATIC_STD_LIBS AND bmt_COMPILER_GCC_TDM)
                target_link_libraries(${target} PRIVATE "-shared-libgcc" "-shared-libstdc++")
            endif()
        elseif(bmt_COMPILER_MSVC)
            if(bmt_USE_STATIC_STD_LIBS)
                set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
            endif()
        endif()
    endif()
endfunction()

function(create_symlink_to_dir target directory_path)
    # Only add the custom command if source directory exists, and symlink target does NOT exist
    if(EXISTS "${directory_path}")
        # Extract the directory name (last component) to use as the symlink name
        get_filename_component(dir_name "${directory_path}" NAME)

        # Compose the destination symlink path using generator expression for target file directory
        set(symlink_path "$<TARGET_FILE_DIR:${target}>/${dir_name}")

        # Unfortunately, generator expressions like $<TARGET_FILE_DIR:...> are evaluated at build time,
        # so we cannot test their existence here at configure time.
        # To avoid duplicate commands, just add the command unconditionally,
        # or consider just issuing the command always since create_symlink is idempotent.

        add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink "${directory_path}" "${symlink_path}"
        )
    endif()
endfunction()

function(copy_binary_output target target_file)
        add_custom_command(
		    TARGET ${target} POST_BUILD
		    COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_FILE:${target_file}>
			$<TARGET_FILE_DIR:${target}>
        )
endfunction()

function(bmt_add_executable target)
    # Parse arguments: options, single-value args, multi-value args
    set(options)
    set(oneValueArgs ASSETS RESOURCES)
    set(multiValueArgs SOURCES DEPENDS INCLUDES CODEGEN)  # CODEGEN — targets this must build after (add_dependencies)
    cmake_parse_arguments(THIS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    source_group("" FILES ${THIS_SOURCES})

    # Add executable or GUI executable if requested
    add_executable(${target} ${THIS_SOURCES})
    if(MSVC)
        set_target_properties(${target} PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
    elseif(CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
        set_target_properties(${target} PROPERTIES LINK_FLAGS -"Wl,/entry:mainCRTStartup")
    endif()

    bmt_set_stdlib(${target})

    # Link dependencies if any
    if(THIS_DEPENDS)
        target_link_libraries(${target} PRIVATE ${THIS_DEPENDS})
        #add_dependencies(${target} ${THIS_DEPENDS})
    endif()
    
    if(THIS_ASSETS)
        create_symlink_to_dir(${target} ${THIS_ASSETS})
    endif()

    # Default include directories
    target_include_directories(${target} PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

    # Optional additional include directories
    if(THIS_INCLUDES)
        target_include_directories(${target} PRIVATE ${THIS_INCLUDES})
    endif()

    # Pre-build code-generation dependencies (e.g. ctxgen)
    if(THIS_CODEGEN)
        add_dependencies(${target} ${THIS_CODEGEN})
    endif()

    target_compile_features(${target} PUBLIC cxx_std_17)

    # On Windows, copy runtime DLLs next to the executable
    if(WIN32 AND BUILD_SHARED_LIBS)
        copy_binary_output(${target} XFG)
    endif()
endfunction()

function(bmt_add_test TEST_NAME)
    # GTEST flag: link GTest + gtest_main and use gtest_discover_tests.
    # Without GTEST: test provides its own main(), registered via add_test.
    cmake_parse_arguments(
        BMT_TEST
        "GTEST"
        ""
        "SOURCES;DEPENDS"
        ${ARGN}
    )

    add_executable(${TEST_NAME} ${BMT_TEST_SOURCES})
    bmt_set_stdlib(${TEST_NAME})

    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )
    target_compile_features(${TEST_NAME} PRIVATE cxx_std_11)

    if(BMT_TEST_GTEST)
        target_link_libraries(${TEST_NAME} PRIVATE
            ${BMT_TEST_DEPENDS}
            GTest::gtest
            GTest::gtest_main
        )
        gtest_discover_tests(${TEST_NAME}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            PROPERTIES TIMEOUT 30
            DISCOVERY_MODE PRE_TEST
        )
    else()
        target_link_libraries(${TEST_NAME} PRIVATE ${BMT_TEST_DEPENDS})
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endif()
endfunction()

function(bmt_find_package)
        set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules/")
        list(GET ARGN 0 target)
        list(REMOVE_AT ARGN 0)

        if(TARGET ${target})
                message(FXFGL_ERROR "Target '${target}' is already defined")
        endif()

        cmake_parse_arguments(THIS "" "" "INCLUDE;LINK" ${ARGN})
        if(THIS_UNPARSED_ARGUMENTS)
                message(FXFGL_ERROR "Unknown arguments when calling bmt_import_library: ${THIS_UNPARSED_ARGUMENTS}")
        endif()

	find_package(${target} REQUIRED)

        add_library(${target} INTERFACE)

        if(THIS_INCLUDE)
                foreach(include_dir IN LISTS "${THIS_INCLUDE}")
                if (NOT include_dir)
                        message(FXFGL_ERROR "No path given for include dir ${THIS_INCLUDE}")
                endif()
                target_include_directories(${target} INTERFACE "$<BUILD_INTERFACE:${include_dir}>")
                endforeach()
        endif()

        if(THIS_LINK)
                foreach(link_item IN LISTS ${THIS_LINK})
                        if(NOT link_item)
                                message(FXFGL_ERROR "Missing item in ${THIS_LINK}")
                        endif()
                        target_link_libraries(${target} INTERFACE "$<BUILD_INTERFACE:${link_item}>")
                endforeach()
        endif()
endfunction()

function(bmt_copy_dll target)

endfunction()

function(bmt_add_sandbox SANDBOX_NAME)
    cmake_parse_arguments(SANDBOX "" "" "SOURCES;DEPENDS" ${ARGN})

    bmt_add_executable(${SANDBOX_NAME}
        ASSETS  "${PROJECT_SOURCE_DIR}/assets"
        SOURCES ${SANDBOX_SOURCES}
        DEPENDS bmt_engine ${SANDBOX_DEPENDS}
    )
endfunction()
