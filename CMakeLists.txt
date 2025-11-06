cmake_minimum_required(VERSION 3.10)
project(S7Server VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/S7Server/snap7)

# Source files
set(SOURCES
    S7Server/main.cpp
)

# Create executable
add_executable(S7Server ${SOURCES})

# Link directories
link_directories(${PROJECT_SOURCE_DIR}/S7Server/snap7)

# Platform-specific settings
if(WIN32)
    # Windows-specific libraries
    target_link_libraries(S7Server 
        ${PROJECT_SOURCE_DIR}/S7Server/snap7/snap7.lib
        ws2_32
        winmm
    )
    
    # Copy DLL to output directory after build
    add_custom_command(TARGET S7Server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/S7Server/snap7/snap7.dll"
        $<TARGET_FILE_DIR:S7Server>
    )
elseif(UNIX)
    # Linux-specific libraries
    # Try to use the static library if available, otherwise use shared library
    # The NAMES order prefers static linking (libsnap7.a) for better portability
    find_library(SNAP7_LIB 
        NAMES libsnap7.a snap7
        PATHS ${PROJECT_SOURCE_DIR}/S7Server/snap7
        NO_DEFAULT_PATH
    )
    
    if(SNAP7_LIB)
        target_link_libraries(S7Server ${SNAP7_LIB} pthread)
    else()
        # Fallback to dynamic linking
        target_link_libraries(S7Server snap7 pthread)
    endif()
    
    # Set RPATH for shared library if needed
    set_target_properties(S7Server PROPERTIES
        BUILD_RPATH "${PROJECT_SOURCE_DIR}/S7Server/snap7"
        INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif()

# Installation
install(TARGETS S7Server DESTINATION bin)

# Install snap7 shared library on Linux
if(UNIX)
    if(EXISTS "${PROJECT_SOURCE_DIR}/S7Server/snap7/libsnap7.so")
        install(FILES "${PROJECT_SOURCE_DIR}/S7Server/snap7/libsnap7.so" 
                DESTINATION lib)
    endif()
endif()

# Print build information
message(STATUS "Building S7Server")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ compiler: ${CMAKE_CXX_COMPILER}")
