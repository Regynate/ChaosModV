cmake_minimum_required(VERSION 3.31)
set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/../vendor)
set(CMAKE_TOOLCHAIN_FILE ${VENDOR_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)
set(VCPKG_TARGET_TRIPLET x64-windows-static)

project(ChaosMod)

file(GLOB ROOT_SRC ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE SRC ${PROJECT_SOURCE_DIR}/Components/*.cpp ${PROJECT_SOURCE_DIR}/Effects/*.cpp
    ${PROJECT_SOURCE_DIR}/Memory/*.cpp ${PROJECT_SOURCE_DIR}/Util/*.cpp)
file(GLOB PATTERNS_SRC ${VENDOR_DIR}/Patterns/Patterns.cpp)
add_library(ChaosMod MODULE ${ROOT_SRC} ${SRC} ${PATTERNS_SRC} ChaosMod.rc)

file (STRINGS ${PROJECT_SOURCE_DIR}/../version.txt MOD_VERSION)
configure_file(${PROJECT_SOURCE_DIR}/InfoTEMPLATE.h.in ${PROJECT_SOURCE_DIR}/Info.h @ONLY)

target_precompile_headers(ChaosMod PUBLIC stdafx.cpp)

set_target_properties(ChaosMod PROPERTIES SUFFIX ".asi")
set_target_properties(ChaosMod PROPERTIES CXX_STANDARD 20)
set_target_properties(ChaosMod PROPERTIES CXX_SCAN_FOR_MODULES OFF)

target_compile_definitions(ChaosMod PUBLIC $<$<CONFIG:Debug>:CHAOSDEBUG>)
target_compile_definitions(ChaosMod PUBLIC NDEBUG)
target_compile_definitions(ChaosMod PUBLIC WIN32_LEAN_AND_MEAN)

set(USE_TLS ON)

find_package(MbedTLS CONFIG REQUIRED)

set(include_dirs ${PROJECT_SOURCE_DIR} ${VENDOR_DIR} ${VENDOR_DIR}/lua/include)
set(link_libs shv minhook lua54 winmm d3dcompiler xinput wsock32 ws2_32 MbedTLS::mbedtls MbedTLS::mbedx509 MbedTLS::mbedcrypto ixwebsocket)

add_subdirectory(${VENDOR_DIR}/shv shv)
add_subdirectory(${VENDOR_DIR}/minhook minhook)

add_subdirectory(${VENDOR_DIR}/IXWebSocket ixwebsocket)

target_include_directories(ChaosMod PUBLIC ${include_dirs})
target_link_directories(ChaosMod PUBLIC ${VENDOR_DIR}/lua)
target_link_libraries(ChaosMod PUBLIC ${link_libs})

if (WITH_DEBUG_PANEL_SUPPORT)
    target_compile_definitions(ChaosMod PUBLIC WITH_DEBUG_PANEL_SUPPORT)
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(ChaosMod PUBLIC /bigobj /Zi /GT /W2 /WX -DUNICODE -D_UNICODE -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
    target_link_options(ChaosMod PUBLIC /DEBUG /OPT:REF /OPT:ICF)
else()
    target_compile_options(ChaosMod PUBLIC -municode -flarge-source-files -Werror -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-variable -Wno-maybe-uninitialized -Wno-array-bounds -Wno-unused-but-set-variable)
    target_link_options(ChaosMod PUBLIC -static)
endif()