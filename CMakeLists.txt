cmake_minimum_required(VERSION 3.25)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL
    "Export compile_commands.json for tooling")

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/RbxLibVersion.cmake")
set(RBXLIB_VERSION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/RbxLibVersion.txt")
rbxlib_read_version_file("${RBXLIB_VERSION_FILE}" RBXLIB_VERSION)
project(RbxLib VERSION "${RBXLIB_VERSION}" LANGUAGES CXX)

include(CTest)
include(FetchContent)

set(RBXLIB_DEFAULT_VCPKG_TRIPLET "x64-windows-static-md")
set(VCPKG_TARGET_TRIPLET "${RBXLIB_DEFAULT_VCPKG_TRIPLET}" CACHE STRING
    "vcpkg triplet used by RbxLib")
set(VCPKG_INSTALLED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed" CACHE PATH
    "Local vcpkg manifest install directory")

set(RBXLIB_VCPKG_PREFIX "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
if(NOT EXISTS "${RBXLIB_VCPKG_PREFIX}/share/nlohmann_json/nlohmann_jsonConfig.cmake")
    message(FATAL_ERROR
        "RbxLib requires nlohmann-json for triplet '${VCPKG_TARGET_TRIPLET}'. "
        "Install it with: vcpkg install --triplet ${VCPKG_TARGET_TRIPLET}")
endif()
list(PREPEND CMAKE_PREFIX_PATH "${RBXLIB_VCPKG_PREFIX}")

find_package(nlohmann_json CONFIG REQUIRED)

option(USE_LOCAL_WINUX "Use local Winux" ON)
if(USE_LOCAL_WINUX)
    get_filename_component(RBXLIB_WINUX_SOURCE_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/../Winux" ABSOLUTE)
    if(NOT EXISTS "${RBXLIB_WINUX_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR
            "USE_LOCAL_WINUX is ON, but no Winux project was found at ${RBXLIB_WINUX_SOURCE_DIR}")
    endif()
    set(VCPKG_INSTALLED_DIR "${RBXLIB_WINUX_SOURCE_DIR}/vcpkg_installed")
    add_subdirectory("${RBXLIB_WINUX_SOURCE_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}/_deps/winux-build")
else()
    FetchContent_Declare(
        winux
        URL https://github.com/Past-Horizon/Winux/archive/refs/tags/v1.0.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(winux)
    set(RBXLIB_WINUX_SOURCE_DIR "${winux_SOURCE_DIR}")
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Sources.cmake")

add_library(RbxLib STATIC ${RBXLIB_SOURCES} ${RBXLIB_HEADERS})

add_library(RbxLib::RbxLib ALIAS RbxLib)

add_custom_command(TARGET RbxLib POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
        "-DRBXLIB_VERSION_FILE=${RBXLIB_VERSION_FILE}"
        "-DRBXLIB_VERSION_LOCK_FILE=${CMAKE_CURRENT_SOURCE_DIR}/out/rbxlib-version.lock"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/AdvanceRbxLibBuildNumber.cmake"
    VERBATIM
)

target_compile_features(RbxLib PUBLIC cxx_std_20)

target_include_directories(RbxLib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(RbxLib
    PUBLIC
        nlohmann_json::nlohmann_json
)

include("${RBXLIB_WINUX_SOURCE_DIR}/cmake/StaticLibMerge.cmake")
add_dependencies(RbxLib Winux)
winux_merge_static_libraries(RbxLib Winux)

option(RBXLIB_BUILD_EXAMPLES "Build RbxLib showcase programs" OFF)
if(RBXLIB_BUILD_EXAMPLES)
    add_executable(RbxLibShowcase examples/Showcase.cpp)
    target_link_libraries(RbxLibShowcase
        PRIVATE
            RbxLib::RbxLib
    )
endif()

if(BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
    include(GoogleTest)

    add_executable(RbxLibTests
        tests/Offsets/OffsetProviderTests.cpp
        tests/DataModel/DataModelTests.cpp
        tests/Rendering/RenderViewTests.cpp
        tests/Instances/InstanceTests.cpp
        tests/Players/PlayerTests.cpp
    )
    target_link_libraries(RbxLibTests
        PRIVATE
            RbxLib::RbxLib
            GTest::gtest_main
            Winux
    )
    gtest_discover_tests(RbxLibTests)

    add_executable(PlayerInstanceTests
        tests/private/PlayerInstanceTests.cpp
    )
    target_link_libraries(PlayerInstanceTests
        PRIVATE
            RbxLib::RbxLib
            Winux
    )

    add_executable(PlayerProportionsTests
        tests/private/PlayerProportionsTests.cpp
    )
    target_link_libraries(PlayerProportionsTests
        PRIVATE
            RbxLib::RbxLib
            Winux
    )
endif()