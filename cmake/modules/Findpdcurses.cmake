# Find pdcurses C Library 

# Look for header files in the project specific include directory
find_path(PDCURSES_INCLUDE_DIR curses.h HINTS ${PROJECT_SOURCE_DIR}/include ${PDCURSES_BASE}/include ${LIBRARY_BASE}/include)

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
find_library(PDCURSES_LIBRARY NAME pdcurses HINTS ${LIBRARY_HINTS} ${PDCURSES_BASE}/lib ${LIBRARY_BASE}/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PDCURSES DEFAULT_MSG PDCURSES_LIBRARY PDCURSES_INCLUDE_DIR)

mark_as_advanced(PDCURSES_INCLUDE_DIR PDCURSES_LIBRARY)

set(PDCURSES_LIBRARIES ${PDCURSES_LIBRARY})
set(PDCURSES_INCLUDE_DIRS ${PDCURSES_INCLUDE_DIR})
