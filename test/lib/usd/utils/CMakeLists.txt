set(TARGET_NAME UsdUtils)
set(PYTHON_TARGET_NAME UsdUtilsTest)

add_executable(${TARGET_NAME})

# -----------------------------------------------------------------------------
# sources
# -----------------------------------------------------------------------------
target_sources(${TARGET_NAME}
    PRIVATE
        main.cpp
        test_DiffCore.cpp
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

# -----------------------------------------------------------------------------
# unit tests
# -----------------------------------------------------------------------------
mayaUsd_add_test(${TARGET_NAME}
    COMMAND $<TARGET_FILE:${TARGET_NAME}>
)

mayaUsd_add_test(${PYTHON_TARGET_NAME}
    PYTHON_COMMAND "python -m unittest discover -s ${CMAKE_CURRENT_SOURCE_DIR}"
    ENV
        "MAYA_PLUG_IN_PATH=${MAYAUTILS_TEST_MAYAPLUGIN_INSTALL_PATH}"
        "PXR_PLUGINPATH_NAME=${ADDITIONAL_PXR_PLUGINPATH_NAME}"
        "PYTHONPATH=${CMAKE_BINARY_DIR}/${INSTALL_DIR_SUFFIX}/lib/usd/utils:${CMAKE_BINARY_DIR}/src:$ENV{PYTHONPATH}"
)

if (TARGET all_tests)
  add_dependencies(all_tests ${TARGET_NAME} ${PYTHON_TARGET_NAME})
endif()