# Find ncurses C Library (depends on the library being in the project directory, not installed elsewhere)

# Look for header files in the project specific include directory
find_path(NCURSES_INCLUDE_DIR ncurses.h HINTS ${PROJECT_SOURCE_DIR}/include ${NCURSES_BASE}/include ${LIBRARY_BASE}/include)

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
find_library(NCURSES_LIBRARY NAMES ncursesw HINTS ${LIBRARY_HINTS} ${NCURSES_BASE}/lib ${LIBRARY_BASE}/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NCURSES DEFAULT_MSG NCURSES_LIBRARY NCURSES_INCLUDE_DIR)

mark_as_advanced(NCURSES_INCLUDE_DIR NCURSES_LIBRARY)

set(NCURSES_LIBRARIES ${NCURSES_LIBRARY})
set(NCURSES_INCLUDE_DIRS ${NCURSES_INCLUDE_DIR})
