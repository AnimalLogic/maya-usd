####################################################################################################
# Setup 
####################################################################################################
cmake_minimum_required(VERSION 2.8.12)
include(RezBuild)
rez_find_packages(PREFIX pkgs AUTO)

list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/defaults
)

add_compile_options(-Wno-deprecated -msse)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PYTHON_LIBRARIES ${python_LIBRARIES})

####################################################################################################
# Setup build to build from maya-usd repo
####################################################################################################
set(AL_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

set (MAYA_USD_WIP_REPO_DIR_REL ${CMAKE_CURRENT_SOURCE_DIR}/../../maya-usd)
get_filename_component(MAYA_USD_LOCAL_REPO_DIR ${MAYA_USD_WIP_REPO_DIR_REL} ABSOLUTE)

include(maya-usd-subproject)

include(${MAYA_USD_SOURCE_DIR}/cmake/utils.cmake)
include(${MAYA_USD_SOURCE_DIR}/cmake/googletest.cmake)
include(${MAYA_USD_SOURCE_DIR}/cmake/test.cmake)

####################################################################################################
# General
####################################################################################################
set(USDTRANSACTION_INCLUDE_LOCATION ${MAYA_USD_SOURCE_DIR}/plugin/al/usdtransaction)

add_subdirectory(${MAYA_USD_SOURCE_DIR}/plugin/al/usdtransaction ${MAYA_USD_BUILD_DIR}/usdtransaction)

add_custom_target(all_tests
  ${CMAKE_CTEST_COMMAND} --output-on-failure
)


####################################################################################################
# Install
####################################################################################################
rez_install_cmake(DESTINATION cmake
    INCLUDE_DIRS include
    LIBRARY_DIRS lib
    LIBRARIES ${LIBRARY_NAME}
    )
