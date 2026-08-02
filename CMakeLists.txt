cmake_minimum_required(VERSION 3.21)
project(gd-ai-mod VERSION 1.0.0)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Geode SDK not found. Install it from https://geode-sdk.org/install and set GEODE_SDK env var.")
endif()

# Work around an upstream fmt/Clang incompatibility: the fmt version pinned
# by the Geode SDK (10.1.1) fails to compile with the newer Clang shipped in
# recent Android NDKs. Fetching a newer fmt here first makes the build reuse
# this version instead of the older, broken one.
set(CPM_DOWNLOAD_VERSION 0.38.1)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
if(NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CPM_DOWNLOAD_LOCATION}
    )
endif()
include(${CPM_DOWNLOAD_LOCATION})

CPMAddPackage(
    NAME fmt
    GITHUB_REPOSITORY fmtlib/fmt
    GIT_TAG 10.2.1
    OPTIONS "FMT_INSTALL OFF"
)

add_subdirectory($ENV{GEODE_SDK} $ENV{GEODE_SDK}/build)

file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(${PROJECT_NAME} geode-sdk)

setup_geode_mod(${PROJECT_NAME})
