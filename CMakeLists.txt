cmake_minimum_required(VERSION 3.15)

set (CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "Configs" FORCE)
set(CMAKE_VERBOSE_MAKEFILE OFF)

# Read version number from command line argument
if(DEFINED LIBHC_VERSION)
    string(REGEX MATCH "v?([0-9]+)\\.([0-9]+)\\.([0-9]+)" _ ${LIBHC_VERSION})
    set(CPACK_PACKAGE_VERSION_MAJOR ${CMAKE_MATCH_1})
    set(CPACK_PACKAGE_VERSION_MINOR ${CMAKE_MATCH_2})
    set(CPACK_PACKAGE_VERSION_PATCH ${CMAKE_MATCH_3})
    set(PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1})
    set(PROJECT_VERSION_MINOR ${CMAKE_MATCH_2})
    set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_3})
else()
    set(CPACK_PACKAGE_VERSION_MAJOR "0")
    set(CPACK_PACKAGE_VERSION_MINOR "0")
    set(CPACK_PACKAGE_VERSION_PATCH "0")
    set(PROJECT_VERSION_MAJOR "0")
    set(PROJECT_VERSION_MINOR "0")
    set(PROJECT_VERSION_PATCH "0")
endif()

# --- Options ---
# Architecture can be: x86_64 (default), i386, arm, aarch64
set(ARCH "x86_64" CACHE STRING "Target architecture for compilation")

# --- Architecture Specific Flags ---
# We use a list for flags to ensure CMake passes them as separate arguments to the compiler
set(ARCH_FLAGS "")

if(ARCH STREQUAL "aarch64")
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    list(APPEND ARCH_FLAGS "-static" "-O0" "-g")
    message(STATUS "Configuring for AArch64 (Cross-compile)")

elseif(ARCH STREQUAL "arm")
    set(CMAKE_C_COMPILER arm-linux-gnueabi-gcc)
    
    # -march=armv5t explicitly restricts the compiler to ancient instructions
    list(APPEND ARCH_FLAGS "-march=armv5t" "-marm" "-static" "-O0" "-g")
    message(STATUS "Configuring for Legacy ARM 32-bit ARMv5t (Cross-compile)")

elseif(ARCH STREQUAL "i386")
    list(APPEND ARCH_FLAGS "-m32" "-static" "-O0" "-g")
    message(STATUS "Configuring for i386 32-bit")
else()
    list(APPEND ARCH_FLAGS "-static" "-O0" "-g")
    message(STATUS "Configuring for Native x86_64")
endif()
project(libhc VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH} LANGUAGES C CXX)

# --- Build Target ---
add_executable(test_hc tests/test_hc.c)

# Include the headers from the local directory
target_include_directories(test_hc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Apply static flags, you need this for PANDA
target_compile_options(test_hc PRIVATE ${ARCH_FLAGS} -Wall -Wextra)
if(ARCH_FLAGS MATCHES "-static")
    target_link_options(test_hc PRIVATE ${ARCH_FLAGS})
endif()

# Set output name based on architecture and place the binary in the tests/ directory
set_target_properties(test_hc PROPERTIES
    OUTPUT_NAME "test_hc_${ARCH}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tests"
)

# --- Installation ---
include(GNUInstallDirs)

# Install the header file, we want this /usr/include/panda/hypercall.h
install(FILES include/hypercall.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" 
        DESTINATION "${CMAKE_INSTALL_DOCDIR}"
        RENAME "copyright")

include(CPackConfig.txt)
