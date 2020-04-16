set(TARGET_NAME DiffCore)

add_executable(${TARGET_NAME})

# -----------------------------------------------------------------------------
# sources
# -----------------------------------------------------------------------------
target_sources(${TARGET_NAME}
    PRIVATE
        main.cpp
        test_DiffCore.cpp
)

# -----------------------------------------------------------------------------
# link libraries
# -----------------------------------------------------------------------------
target_link_libraries(${TARGET_NAME}
    PRIVATE 
        GTest::GTest
        mayaUsdUtils
)

# -----------------------------------------------------------------------------
# environment variables
# -----------------------------------------------------------------------------
add_definitions(
  -DAL_EXTRAS_TEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

# -----------------------------------------------------------------------------
# unit tests
# -----------------------------------------------------------------------------
mayaUsd_add_test(${TARGET_NAME}
    COMMAND $<TARGET_FILE:${TARGET_NAME}>
)

set(TARGET_NAME MayaTransformAPI)

add_executable(${TARGET_NAME})

# -----------------------------------------------------------------------------
# sources
# -----------------------------------------------------------------------------
target_sources(${TARGET_NAME}
    PRIVATE
        main.cpp
        test_MayaTransformAPI.cpp
)

# -----------------------------------------------------------------------------
# link libraries
# -----------------------------------------------------------------------------
target_link_libraries(${TARGET_NAME}
    PRIVATE 
        GTest::GTest
        mayaUsdUtils
)

# -----------------------------------------------------------------------------
# environment variables
# -----------------------------------------------------------------------------
add_definitions(
  -DAL_EXTRAS_TEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

mayaUsd_copyDirectory(${TARGET_NAME} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
                             DIRECTORY data)
# -----------------------------------------------------------------------------
# unit tests
# -----------------------------------------------------------------------------
mayaUsd_add_test(${TARGET_NAME}
    COMMAND $<TARGET_FILE:${TARGET_NAME}>
)

list(APPEND test_script_files
    test_MayaTransformAPI.py
)
                             
foreach(script ${test_script_files})
    mayaUsd_get_unittest_target(target ${script})
    mayaUsd_add_test(${target}
        PYTHON_MODULE ${target}
        ENV
            "MAYA_PLUG_IN_PATH=${MAYAUTILS_TEST_MAYAPLUGIN_INSTALL_PATH}"
    )

    # Assign a ctest label to these tests for easy filtering.
    set_tests_properties(${target} PROPERTIES LABELS ufe)
endforeach()
