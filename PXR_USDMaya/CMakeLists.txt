project(PXR_USDMaya)
cmake_minimum_required(VERSION 2.8.8)

include(RezBuild)

rez_find_packages(PREFIX pkgs AUTO)

####################################################################################################
# Setup build to build from maya-usd repo
####################################################################################################

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

set (MAYA_USD_WIP_REPO_DIR_REL ${CMAKE_CURRENT_SOURCE_DIR}/../../maya-usd)
get_filename_component(MAYA_USD_WIP_REPO_DIR ${MAYA_USD_WIP_REPO_DIR_REL} ABSOLUTE)

include(maya-usd-subproject)

####################################################################################################


# By default pixar bakes RPATH, skip that.
set(CMAKE_SKIP_RPATH  TRUE)

# pxr cmake files search path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/defaults
                      ${CMAKE_SOURCE_DIR}/cmake/modules
                      ${CMAKE_SOURCE_DIR}/cmake/macros)

# AL cmake files search path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../cmake
	)
	

include(ALRezBuild)

include(Options)
include(ProjectDefaults)

# pxr python shebang setup
find_package(PythonInterp)
set(PXR_PYTHON_SHEBANG "${PYTHON_EXECUTABLE}"
    CACHE
    STRING
    "Replacement path for Python #! line."
)

find_package(Maya REQUIRED)
# Override MAYA_BASE_DIR to make tests pass
# Since maya-2017.0.al4.10.2 mayapy is not part of mayaDevKit
unset(MAYA_BASE_DIR CACHE)
set(MAYA_BASE_DIR $ENV{REZ_AL_MAYA_STARTUP_ROOT})

# pxr tests use MAYA_LOCATION cmake variable
set(MAYA_LOCATION $ENV{MAYA_LOCATION})

find_package(GLUT REQUIRED)
set(BOOST_ROOT ${boost_ROOT})
# because boost and boost_python are separate
if(boost_LIBRARY_DIRS AND boost_python_LIBRARY_DIRS)
    set(Boost_LIBRARY_DIR ${boost_LIBRARY_DIRS} ${boost_python_LIBRARY_DIRS})
elseif(AL_boost_LIBRARY_DIRS AND AL_boost_python_LIBRARY_DIRS)
    set(Boost_LIBRARY_DIR ${AL_boost_LIBRARY_DIRS} ${AL_boost_python_LIBRARY_DIRS})
endif()
find_package( Boost
	COMPONENTS
        iostreams
        python
        regex
        system
        program_options
    REQUIRED
)

# boost
add_definitions("-DBOOST_NAMESPACE=${boost_NAMESPACE}")

# CXXDefaults will set a variety of variables for the project.
# Consume them here. This is an effort to keep the most common
# build files readable.
include(CXXDefaults)
add_definitions(${_PXR_CXX_DEFINITIONS})
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6")
    set(CMAKE_CXX_FLAGS ${_PXR_CXX_FLAGS})
else()
    string(STRIP ${_PXR_CXX_FLAGS} _PXR_CXX_FLAGS_LIST)
    string(REPLACE " " ";" _PXR_CXX_FLAGS_LIST ${_PXR_CXX_FLAGS_LIST})
    foreach(pxr_flag IN LISTS _PXR_CXX_FLAGS_LIST)
        if(pxr_flag STREQUAL "-std=c++11")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -D_GLIBCXX_USE_CXX11_ABI=0")
        else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${pxr_flag}")
        endif()
    endforeach()
endif()

# to have LD_LIBRARY_PATH overriding RPATH
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--enable-new-dtags")

include(Public)

pxr_toplevel_prologue()
pxr_core_prologue()

add_subdirectory(maya)

al_setup_python()
pxr_setup_plugins()

add_custom_target(all_tests
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_tests.sh ${CMAKE_INSTALL_PREFIX}
)

rez_install_cmake(DESTINATION cmake
    INCLUDE_DIRS third_party/maya/include
    LIBRARY_DIRS third_party/maya/lib)
