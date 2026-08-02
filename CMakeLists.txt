cmake_minimum_required(VERSION 3.21)
project(gd-ai-mod VERSION 1.0.0)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Geode SDK not found. Install it from https://geode-sdk.org/install and set GEODE_SDK env var.")
endif()

# Work around a Clang/NDK bug: the very new Clang shipped with recent
# Android NDKs can't correctly evaluate fmt's compile-time format-string
# checking (consteval), causing build failures deep inside the fmt library.
# This disables that compile-time check only — format strings are still
# checked normally at runtime, so nothing else is affected.
add_compile_definitions(FMT_USE_CONSTEVAL=0)

add_subdirectory($ENV{GEODE_SDK} $ENV{GEODE_SDK}/build)

file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(${L} SHARED ${SOURCES})

target_link_libraries(${L} geode-sdk)

setup_geode_mod(${L})
