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



####################################################################################################
# Boost
####################################################################################################

set(BOOST_ROOT ${boost_ROOT})

IF(DEFINED boost_NAMESPACE)
  add_definitions("-DBOOST_NAMESPACE=${boost_NAMESPACE}")
  set(Boost_NAMESPACE ${boost_NAMESPACE}) #Used by newer versions of FindBoost
ELSE()
  add_definitions("-DBOOST_NAMESPACE=boost")
ENDIF()

IF(NOT DEFINED BOOST_LIBRARY_DIRS)
  IF(boost_LIBRARY_DIRS AND boost_python_LIBRARY_DIRS)
      set(Boost_LIBRARY_DIR ${boost_LIBRARY_DIRS} ${boost_python_LIBRARY_DIRS})
  ELSEIF(AL_boost_LIBRARY_DIRS AND AL_boost_python_LIBRARY_DIRS)
      set(Boost_LIBRARY_DIR ${AL_boost_LIBRARY_DIRS} ${AL_boost_python_LIBRARY_DIRS})
  ENDIF()
ENDIF()

IF(NOT DEFINED BOOST_INCLUDEDIR)
    set(BOOST_INCLUDEDIR ${AL_boost_INCLUDE_DIRS})
ENDIF()

SET(Boost_DEBUG OFF) #Use this to debug FindBoost

# FindBoost is particularly buggy, and doesn't like custom boost locations.
# Adding specific components forces calls to _Boost_find_library, which
# is the rationale for listing them here.
find_package(Boost COMPONENTS
                python # initialises Boost_PYTHON_LIBRARY
                thread # initialises Boost_THREAD_LIBRARY
                REQUIRED
)

####################################################################################################
# General
####################################################################################################
set(USDUTILS_INCLUDE_LOCATION ${MAYA_USD_SOURCE_DIR}/lib/usd/utils)

add_subdirectory(${MAYA_USD_SOURCE_DIR}/lib/usd/utils ${MAYA_USD_BUILD_DIR}/usdutils)


####################################################################################################
# Test
####################################################################################################

include(${MAYA_USD_SOURCE_DIR}/cmake/test.cmake)
include(${MAYA_USD_SOURCE_DIR}/cmake/googletest.cmake)
fetch_googletest()
enable_testing()
add_subdirectory(${MAYA_USD_SOURCE_DIR}/test/lib/usd/utils ${MAYA_USD_BUILD_DIR}/test)
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
