set(TARGET_NAME "mayaUsdUtils")

add_library(${TARGET_NAME} SHARED)

set(MAYAUSDUTILS_PYTHON_LIBRARY_NAME ${USDUTILS_LIBRARY_NAME}Python)
set(MAYAUSDUTILS_PYTHON_LIBRARY_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/python/mayaUsdUtils)

# -----------------------------------------------------------------------------
# sources
# -----------------------------------------------------------------------------
target_sources(${TARGET_NAME}
    PRIVATE
        MayaTransformAPI.cpp
        DebugCodes.cpp
        DiffCore.cpp
        wrapMayaTransformAPI.cpp
        module.cpp
)

# -----------------------------------------------------------------------------
# compile definitions
# -----------------------------------------------------------------------------
target_compile_definitions(${TARGET_NAME}
    PRIVATE
        MAYA_USD_UTILS_EXPORT
)
 
# -----------------------------------------------------------------------------
# include directories
# -----------------------------------------------------------------------------
target_include_directories(${TARGET_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${USD_INCLUDE_DIR}
        ${CMAKE_BINARY_DIR}/include
)

# -----------------------------------------------------------------------------
# link libraries
# -----------------------------------------------------------------------------
target_link_libraries(${TARGET_NAME}
    PUBLIC
        gf
        usd
)

# -----------------------------------------------------------------------------
# promote headers
# -----------------------------------------------------------------------------
set(headers
    ALHalf.h
    Api.h
    DebugCodes.h
    DiffCore.h
    ForwardDeclares.h
    MayaTransformAPI.h
    SIMD.h
)

mayaUsd_promoteHeaderListWithSubdir( 
    HEADERS 
        ${headers}
    BASE_PATH ${TARGET_NAME}
)

# -----------------------------------------------------------------------------
# install
# -----------------------------------------------------------------------------
install(TARGETS ${TARGET_NAME}
    LIBRARY
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    RUNTIME
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

install(FILES ${headers}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${TARGET_NAME}
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    install(FILES $<TARGET_PDB_FILE:${TARGET_NAME}> 
            DESTINATION ${CMAKE_INSTALL_PREFIX}/lib OPTIONAL
    )
endif()

list(APPEND mayausdutils_python_source
  module.cpp
  wrapMayaTransformAPI.cpp
)

add_library(${MAYAUSDUTILS_PYTHON_LIBRARY_NAME}
  SHARED
    ${mayausdutils_python_source}
)

target_compile_definitions(${MAYAUSDUTILS_PYTHON_LIBRARY_NAME}
  PRIVATE
    MFB_PACKAGE_NAME=mayaUsdUtils
    MFB_ALT_PACKAGE_NAME=mayaUsdUtils
    MFB_PACKAGE_MODULE=mayaUsdUtils
)

target_include_directories(${MAYAUSDUTILS_PYTHON_LIBRARY_NAME} 
    PUBLIC
    ${UTILS_INCLUDE_LOCATION} 
    ${MAYAUSDUTILS_INCLUDE_LOCATION} 
    ${USD_INCLUDE_DIR}
    )

target_link_libraries(${MAYAUSDUTILS_PYTHON_LIBRARY_NAME}
  ${USDUTILS_LIBRARY_NAME}
  ${python_LIBRARIES}
  tf
  gf
  usd
  usdGeom
)

set_target_properties(${MAYAUSDUTILS_PYTHON_LIBRARY_NAME}
  PROPERTIES
    PREFIX "_"
    SUFFIX ".so"
    OUTPUT_NAME "mayaUsdUtils"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(tests)


# install python library
install(TARGETS ${MAYAUSDUTILS_PYTHON_LIBRARY_NAME}
  LIBRARY
  DESTINATION ${MAYAUSDUTILS_PYTHON_LIBRARY_LOCATION}
  RUNTIME
  DESTINATION ${MAYAUSDUTILS_PYTHON_LIBRARY_LOCATION}
)