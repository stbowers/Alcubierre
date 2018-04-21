# Find zlib C Library 

# Look for header files in the project specific include directory
find_path(ZLIB_INCLUDE_DIR zlib.h HINTS ${PROJECT_SOURCE_DIR}/include ${ZLIB_BASE}/include ${LIBRARY_BASE}/include)

# Look for library in the project specific library path (We're specifically looking for the static library here)
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(LIBRARY_HINTS ${PROJECT_SOURCE_DIR}/lib/Win64)
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LIBRARY_HINTS ${PROJECT_SOURCE_DIR}/lib/Linux)
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(LIBRARY_HINTS ${PROJECT_SOURCE_DIR}/lib/macOS)
endif()
find_library(ZLIB_LIBRARY NAME z zlib HINTS ${LIBRARY_HINTS} ${ZLIB_BASE}/lib ${LIBRARY_BASE}/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZLIB DEFAULT_MSG ZLIB_LIBRARY ZLIB_INCLUDE_DIR)

mark_as_advanced(ZLIB_INCLUDE_DIR ZLIB_LIBRARY)

set(ZLIB_LIBRARIES ${ZLIB_LIBRARY})
set(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR})
