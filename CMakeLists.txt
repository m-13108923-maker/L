cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "x86_64")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(GDAIMod VERSION 1.0.0)

# ── Geode SDK ──────────────────────────────────────────────────────────────────
# Download the Geode SDK if not already present:
#   cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Set GEODE_SDK environment variable to the Geode SDK path")
endif()
set(GEODE_SDK $ENV{GEODE_SDK})
add_subdirectory(${GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# ── Sources ────────────────────────────────────────────────────────────────────
file(GLOB_RECURSE SOURCES src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src)

# ── Link ───────────────────────────────────────────────────────────────────────
target_link_libraries(${PROJECT_NAME} geode-sdk)

# Windows: link ws2_32 and winhttp for HTTP requests
if (WIN32)
    target_link_libraries(${PROJECT_NAME} winhttp ws2_32)
endif()

setup_geode_mod(${PROJECT_NAME})
