# CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

project(UniversalBootstrapProject LANGUAGES CXX)

# Match the VS project's C++20 usage.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Release when using single-config generators.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Keep output binaries in a predictable place.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_executable(UniversalBootstrap
        UniversalBootstrap.cpp
        Resource.rc
)

add_executable(UniversalTester
        UniversalTester/UniversalTester.cpp
        Resource.rc
)

# MSVC-specific compile options roughly aligned with the .vcxproj settings.
if(MSVC)
    target_compile_options(UniversalBootstrap PRIVATE /W3 /permissive-)
    target_compile_options(UniversalTester PRIVATE /W3 /permissive-)

    # Optional: define UNICODE for consistency with the VS project CharacterSet=Unicode.
    target_compile_definitions(UniversalBootstrap PRIVATE UNICODE _UNICODE)
    target_compile_definitions(UniversalTester PRIVATE UNICODE _UNICODE)
endif()