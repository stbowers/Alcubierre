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
	
	# LIBRARY_BASE is the last place our custom .cmake files will look for libraries. This should point to where most libraries are (root of lib/ and include/)
	set(LIBRARY_BASE "C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC")
	# Boost library bases (they're not in the directory listed above)
	set(BOOST_SYSTEM_BASE C:/boost)
	set(BOOST_FILESYSTEM_BASE C:/boost)
	
    # Load packages for libraries we use
    find_package(OpenGL)
    find_package(Vulkan)
    find_package(GLEW)
    find_package(GLFW)
    find_package(lua)
    find_package(boost_system)
    find_package(boost_filesystem)
    set(PROJECT_LIBRARIES ${OPENGL_LIBRARY} ${VULKAN_LIBRARY} ${GLEW_LIBRARY} ${GLFW_LIBRARY} ${LUA_LIBRARY} ${BOOST_SYSTEM_LIBRARY} ${BOOST_FILESYSTEM_LIBRARY})
    set(PROJECT_INCLUDE_DIRS C:/VulkanSDK/1.0.65.1/Include ${OPENGL_INCLUDE_DIR} ${VULKAN_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${LUA_INCLUDE_DIR} ${BOOST_SYSTEM_INCLUDE_DIR} ${BOOST_FILESYSTEM_INCLUDE_DIR})
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # GNU/Linux settings
    add_definitions(-D__LINUX__)
    add_definitions(-D__UNIX__)
    set(CMAKE_C_FLAGS "-Wall -g -lm -finput-charset=UTF-8 -pthread")

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
