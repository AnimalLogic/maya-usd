#We're not using  the opensource maya-usd root cmake file at https://github.com/Autodesk/maya-usd/blob/dev/CMakeLists.txt directly
#As we need to process our own Rez package stuff and not have it overriden by the the adsk file
#This file sets up the the various cmake vars, then calls the cmake files for the subprojects 

#This is a modified version of the cmake file from usd/sdk/maya/CMakeLists.txt
cmake_minimum_required(VERSION 3.13...3.17)

#@todo @eoinm: Setting these was required to get the build to work with CMake 3.11 but I should fix the underlying problem 
cmake_policy(SET CMP0079 OLD)
cmake_policy(SET CMP0026 OLD)



####################################################################################################
# Setup build to build from maya-usd repo
####################################################################################################

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

set (MAYA_USD_WIP_REPO_DIR_REL ${CMAKE_CURRENT_SOURCE_DIR}/../../maya-usd)
get_filename_component(MAYA_USD_LOCAL_REPO_DIR ${MAYA_USD_WIP_REPO_DIR_REL} ABSOLUTE)

include(maya-usd-subproject)


#################################################
# General cmake setup
#################################################
#set cmake modules - ";" separated not ":"!
list(APPEND CMAKE_MODULE_PATH
    ${MAYA_USD_SOURCE_DIR}/cmake
    ${MAYA_USD_SOURCE_DIR}/cmake/defaults
    ${MAYA_USD_SOURCE_DIR}/cmake/modules
)

include(test)

####################################################################################################
option(BUILD_MAYAUSD_LIBRARY "Build Core usdMaya library." ON) #This probably shouldnt be an option as the build will fail otherwise
set(BUILD_USDMAYA_PXR_TRANSLATORS OFF CACHE BOOL "disable pixar shape translator in AL_USDMaya" )

set(MAYA_Foundation_LIBRARY Foundation)
set(MAYA_OpenMaya_LIBRARY OpenMaya)
set(MAYA_OpenMayaAnim_LIBRARY OpenMayaAnim)
set(MAYA_OpenMayaUI_LIBRARY OpenMayaUI)
set(MAYA_OpenMayaRender_LIBRARY OpenMayaRender)
    
set(UFE_LIB_ROOT $ENV{REZ_MAYADEVKIT_ROOT}/maya/devkitBase/devkit/ufe)
set(UFE_INCLUDE_ROOT $ENV{REZ_MAYADEVKIT_ROOT}/maya/devkitBase/devkit/ufe)

find_package(UFE)

include ( RezBuild )
rez_find_packages ( PREFIX pkgs AUTO )

# Bake version
string(REPLACE "." ";" AL_USDMAYA_VERSION $ENV{REZ_BUILD_PROJECT_VERSION})
list(GET AL_USDMAYA_VERSION 0 AL_USDMAYA_VERSION_MAJOR)
list(GET AL_USDMAYA_VERSION 1 AL_USDMAYA_VERSION_MINOR)
list(GET AL_USDMAYA_VERSION 2 AL_USDMAYA_VERSION_PATCH)
configure_file(Version.h.in ${MAYA_USD_AL_PLUGIN_DIR}/lib/AL_USDMaya/AL/usdmaya/Version.h)

#Extract the USD_VERSION_NUM as used in the code from our USDBASE version
math(EXPR USD_VERSION_NUM "$ENV{REZ_USDBASE_MINOR_VERSION} * 100 + $ENV{REZ_USDBASE_PATCH_VERSION}")
MESSAGE("USD_VERSION IS ${USD_VERSION_NUM}")

####################################################################################################
# Boost
####################################################################################################

set(BOOST_ROOT ${boost_ROOT})

IF(DEFINED boost_NAMESPACE)
  add_definitions("-DBOOST_NAMESPACE=${boost_NAMESPACE}")
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

# FindBoost is particularly buggy, and doesn't like custom boost locations.
# Adding specific components forces calls to _Boost_find_library, which
# is the rationale for listing them here.
find_package(Boost COMPONENTS
                python # initialises Boost_PYTHON_LIBRARY
                thread # initialises Boost_THREAD_LIBRARY
                REQUIRED
)

if((CMAKE_COMPILER_IS_GNUCC AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.3)) OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5)))
    set(NEED_BOOST_FILESYSTEM ON)
    find_package(Boost COMPONENTS
                    filesystem # initialises Boost_FILESYSTEM_LIBRARY
                    REQUIRED)
endif()

####################################################################################################

add_definitions("-DAL_MAYA_ENABLE_SIMD=1")
add_definitions(-DLINUX)
add_definitions("-DUSD_VERSION_NUM=${USD_VERSION_NUM}") 



# to get PYTHON_EXECUTABLE
find_package(PythonInterp)
set(Python_EXECUTABLE ${PYTHON_EXECUTABLE})


set(CMAKE_INCLUDE_CURRENT_DIR ON)
include(CXXDefaults)

add_definitions(${_PXR_CXX_DEFINITIONS})

find_package(Maya REQUIRED)
set(PYTHON_LIBRARY $ENV{MAYA_LOCATION}/lib/libpython2.7.so CACHE STRING "" FORCE)
find_package(PythonLibs)


set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_CXX_FLAGS
    -std=c++11
    -msse3
    -Werror
    "${_PXR_CXX_WARNING_FLAGS}"
    "${_PXR_CXX_FLAGS}"
)

if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6)
    list(APPEND CMAKE_CXX_FLAGS -D_GLIBCXX_USE_CXX11_ABI=0)
endif()
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

option(SKIP_USDMAYA_TESTS "Skip test building" OFF)

if(NOT SKIP_USDMAYA_TESTS)
	MESSAGE("TESTING  MAYA_PLUG_IN_PATH $ENV{MAYA_PLUG_IN_PATH}")
    enable_testing()
endif()

# Path to the input schemas templates
set(USD_SCHEMA_PATH ${PXR_CMAKE_DIR}/schemas/usd/schema.usda)

# This will drive the schemas library and binding library names
set(TRANSLATORS_PACKAGE AL_USDMayaTranslators_opensource)

set(AL_USDMAYA_LOCATION_NAME
    "AL_USDMAYA_LOCATION"
    CACHE
    STRING
    "Name of the environment variable used to store AL_USDMaya installation location"
)

set(INSTALL_DIR_SUFFIX plugin/al)
set(AL_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/${INSTALL_DIR_SUFFIX})

# Build our plugs... non-standard as we're building from another source tree
set(EVENTS_INCLUDE_LOCATION ${MAYA_USD_AL_PLUGIN_DIR}/utils)
set(USDUTILS_INCLUDE_LOCATION ${MAYA_USD_AL_PLUGIN_DIR}/usdutils)
set(MAYAUTILS_INCLUDE_LOCATION ${MAYA_USD_AL_PLUGIN_DIR}/mayautils)
set(USDMAYAUTILS_INCLUDE_LOCATION ${MAYA_USD_AL_PLUGIN_DIR}/usdmayautils)
set(MAYATEST_INCLUDE_LOCATION ${MAYA_USD_AL_PLUGIN_DIR}/mayatest)

# pxrUsd translator uses this
set(PROJECT_SOURCE_DIR ${MAYA_USD_AL_PLUGIN_DIR})
# Build usdMaya pxr translator
set(USD_MAYA_ROOT ${usdMaya_ROOT})
set(USD_ROOT ${usdBase_ROOT})
find_package(USD)

include(${MAYA_USD_SOURCE_DIR}/cmake/utils.cmake)



#@note: We could avoid all of this by just calling FetchContent_MakeAvailable() or similar when we fetch the repo
# but for now we're not building everything (e.g pxr plugin) and need a bit more control??
if (BUILD_MAYAUSD_LIBRARY)
    add_subdirectory(${MAYA_USD_SOURCE_DIR}/lib ${MAYA_USD_BUILD_DIR}/maya_usd_lib) #@todo not sure of this suffix..
endif()

add_subdirectory(${MAYA_USD_AL_PLUGIN_DIR}/utils ${MAYA_USD_BUILD_DIR}/utils)
if(NOT SKIP_USDMAYA_TESTS)
	add_subdirectory(${MAYA_USD_AL_PLUGIN_DIR}/mayatest ${MAYA_USD_BUILD_DIR}/mayatest)
	set(MAYA_LIBRARY_DIR ${mayaDevKit_LIBRARY_DIRS})
endif()

add_subdirectory(${MAYA_USD_AL_PLUGIN_DIR}/mayautils ${MAYA_USD_BUILD_DIR}/mayautils)
add_subdirectory(${MAYA_USD_AL_PLUGIN_DIR}/usdmayautils ${MAYA_USD_BUILD_DIR}/usdmayautils)
add_subdirectory(${MAYA_USD_AL_PLUGIN_DIR}/lib ${MAYA_USD_BUILD_DIR}/lib)
add_subdirectory(${MAYA_USD_AL_PLUGIN_DIR}/plugin ${MAYA_USD_BUILD_DIR}/plugin)
add_subdirectory(${MAYA_USD_AL_PLUGIN_DIR}/schemas ${MAYA_USD_BUILD_DIR}/schemas)
add_subdirectory(${MAYA_USD_AL_PLUGIN_DIR}/translators ${MAYA_USD_BUILD_DIR}/translators)



# The opensource build has these custom SdfMetadata in the schema plugInfo.json:

#   - al_usdmaya_mergedTransform
#   - al_usdmaya_selectability
#   - al_usdmaya_lock
#   - al_usdmaya_importAsNative
#   - al_usdmaya_excludeFromProxyShape
#   - al_usdmaya_transformType
#   - al_usdmaya_*
# Internally, these metadatas have been kept in AL_USDCommonSchemas for the cases we want to access them
# without depending on AL_USDMaya therefore we're going to filter these metadatas out.
set(to_filter
    ${MAYA_USD_BUILD_DIR}/schemas/AL/usd/schemas/maya/plugInfo.json
)

message(
    STATUS
    "Removing opensource metadatas from ${to_filter}"
)

execute_process(
    COMMAND
        python
        ${CMAKE_CURRENT_SOURCE_DIR}/schemas/filter_pluginfo.py
        ${to_filter}
    RESULT_VARIABLE
        filter_pluginfo
)

if(filter_pluginfo)
    message(
        FATAL_ERROR
        "An error occured while filtering ${to_filter}"
    )
endif()

IF ($ENV{REZ_BUILD_VARIANT_NUMBER} STREQUAL "0")
    set(doxygen_dirs
        ${MAYA_USD_AL_PLUGIN_DIR}/lib/AL_USDMaya/AL
        ${MAYA_USD_AL_PLUGIN_DIR}/doxygen
    )

    rez_install_doxygen(
        doc
        FILES             ${doxygen_dirs}
        DESTINATION       doc
        DOXYFILE          ${MAYA_USD_AL_PLUGIN_DIR}/doxygen/Doxyfile
        GENERATE_TAGFILE
        USE_TAGFILES
        INSTALL_DOXYFILE
        SKIP_INSTALL
    )
    push_docs_to_gh_pages(
    	REPOSITORY AL_USDMaya
    )
ELSE()
  add_custom_target(doc)
ENDIF()

install(FILES cmake/AL_USDMaya.cmake DESTINATION cmake)

get_property(LIBRARY_LOCATION GLOBAL PROPERTY GLOBAL_LIBRARY_LOCATION)
get_property(PYTHON_LIBRARY_LOCATION GLOBAL PROPERTY GLOBAL_PYTHON_LIBRARY_LOCATION)
configure_file(${MAYA_USD_AL_PLUGIN_DIR}/ALUsdMayaConfig.cmake.in ${PROJECT_BINARY_DIR}/ALUsdMayaConfig.cmake @ONLY)

install(FILES ${PROJECT_BINARY_DIR}/ALUsdMayaConfig.cmake
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  COMPONENT dev
)


add_custom_target(all_tests
    ${CMAKE_CTEST_COMMAND} --output-on-failure
)


rez_install_files(mel/${MAYA_MAJOR_VERSION}/al_usdmaya_autoload.mel RELATIVE mel/${MAYA_MAJOR_VERSION} DESTINATION mel)
