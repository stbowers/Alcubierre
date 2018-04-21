cmake_minimum_required (VERSION 2.6)
project (Alcubierre)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

# Find all source files, including any in subdirectories of src/
file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.c")

# Include files in ./include for the include path
include_directories(${PROJECT_SOURCE_DIR}/include)

# Set up unique settings for each platform
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    # Windows settings
    add_definitions(-D__WIN32__)
    set(CMAKE_C_FLAGS "")

    # LIBRARY_BASE is where all our custom find{package}.cmake files will look for the libraries, after all local locations
    set(LIBRARY_BASE "C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC")

    # Tell cmake where to look for ncurses (pdcurses)
    set(PDCURSES_BASE C:/pdcurs34)
    set(ZLIB_BASE C:/zlib1211)

    # Load packages for libraries we use
    find_package(pdcurses)
    find_package(zlib)
    set(PROJECT_LIBRARIES ${PDCURSES_LIBRARY} ${ZLIB_LIBRARY})
    set(PROJECT_INCLUDE_DIRS ${PDCURSES_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # GNU/Linux settings
    add_definitions(-D__LINUX__)
    add_definitions(-D__UNIX__)
    set(CMAKE_C_FLAGS "-Wall -g -lm -finput-charset=UTF-8 -pthread -O0")

    # LIBRARY_BASE is where all our custom find{package}.cmake files will look for the libraries, after all local locations
    set(LIBRARY_BASE /usr)

    # Load packages for libraries we use
    find_package(ncursesw)
    find_package(panelw)
    find_package(ZLIB)
    set(PROJECT_LIBRARIES ${NCURSES_LIBRARY} ${PANEL_LIBRARY} ${ZLIB_LIBRARY})
    set(PROJECT_INCLUDE_DIRS ${NCURSES_INCLUDE_DIR} ${PANEL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # macOS settings
    add_definitions(-D__MACOS__)
    add_definitions(-D__UNIX__)
    set(CMAKE_C_FLAGS "-Wall -g -lm -finput-charset=UTF-8 -pthread")
    
    # LIBRARY_BASE is where all our custom find{package}.cmake files will look for the libraries, after all local locations
    set(LIBRARY_BASE /opt/local)

    # Load packages for libraries we use
    # macos' ncurses library already has wide char functionality, so no need to link ncursesw
    find_package(ncurses)
    find_package(panel)
    find_package(ZLIB)
    set(PROJECT_LIBRARIES ${NCURSES_LIBRARY} ${PANEL_LIBRARY} ${ZLIB_LIBRARY})
    set(PROJECT_INCLUDE_DIRS ${NCURSES_INCLUDE_DIR} ${PANEL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
endif()

# Set up include directories for header files
include_directories(${PROJECT_INCLUDE_DIRS})

# Compile sources into the binary
add_executable(${PROJECT_NAME} ${SOURCES})

# Link the binary with all needed libs
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBRARIES})
