# Find sfml audio C Library (depends on the library being in the project directory, not installed elsewhere)

# Look for header files in the project specific include directory
find_path(CSFML_INCLUDE_DIR SFML/Audio.h HINTS ${PROJECT_SOURCE_DIR}/include ${CSFML_BASE}/include ${LIBRARY_BASE}/include)

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
find_library(CSFML_LIBRARY NAMES csfml-audio HINTS ${LIBRARY_HINTS} ${CSFML_BASE}/lib ${LIBRARY_BASE}/lib)
find_library(CSFML_SYSTEM_LIBRARY NAMES csfml-system HINTS ${LIBRARY_HINTS} ${CSFML_BASE}/lib ${LIBRARY_BASE}/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CSFML DEFAULT_MSG CSFML_LIBRARY CSFML_INCLUDE_DIR)

mark_as_advanced(CSFML_INCLUDE_DIR CSFML_LIBRARY)

set(CSFML_LIBRARIES ${CSFML_LIBRARY} ${CSFML_SYSTEM_LIBRARY})
set(CSFML_INCLUDE_DIRS ${CSFML_INCLUDE_DIR})
