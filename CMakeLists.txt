cmake_minimum_required(VERSION 3.28.3)
project(Mahl VERSION 1.0 DESCRIPTION "Project of PAA" LANGUAGES CXX)

# Set the C++ language standard.
#
# Using C++23.
set(CMAKE_CXX_STANDARD 23)
# Require the compiler to support exactly this standard.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Flag to set the binaries output directory.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Flag to generate compile_commands.json (Useful for clangd).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Release) # for fastier math calcs
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto -ffast-math")
find_package(OpenMP REQUIRED) # lib to divide the evolve's loop into threads

# Adds the includes directory.
include_directories(include libs)

# Recursively searches for all .cpp files in src/ and subdirectories.
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    libs/*.cpp
)

# Create the executable called 'BinPack' using the source files found.
add_executable(BinPack Main.cpp ${SOURCES})

# Adds the include directories to the 'BinPack' target.
target_include_directories(BinPack PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)


# Set compilation flags.
target_compile_options(BinPack PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wno-deprecated
    -O3
)

# option(RT3_DEBUG "Enables debug logs." OFF)

# if(RT3_DEBUG)
#     target_compile_definitions(BinPack PRIVATE RT3_DEBUG_MODE=1)
# endif()