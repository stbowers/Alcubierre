# Find panel C Library (part of ncurses)

# Look for header files in the project specific include directory
find_path(PANEL_INCLUDE_DIR panel.h HINTS ${PROJECT_SOURCE_DIR}/include ${NCURSES_BASE}/include ${LIBRARY_BASE}/include)

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
find_library(PANEL_LIBRARY NAMES panelw HINTS ${LIBRARY_HINTS} ${NCURSES_BASE}/lib ${LIBRARY_BASE}/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PANEL DEFAULT_MSG PANEL_LIBRARY PANEL_INCLUDE_DIR)

mark_as_advanced(PANEL_INCLUDE_DIR PANEL_LIBRARY)

set(PANEL_LIBRARIES ${PANEL_LIBRARY})
set(PANEL_INCLUDE_DIRS ${PANEL_INCLUDE_DIR})
