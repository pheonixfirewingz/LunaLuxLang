cmake_minimum_required(VERSION 3.22.1)
project(LunaLuxLang CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include(Util)
add_definitions(-DPROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Debug mode by default.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG_MODE)
endif()

if(CMAKE_SYSTEM_NAME MATCHES Linux)
    message(INFO "[${CMAKE_PROJECT_NAME}] - building for Linux....")
    add_definitions(-DON_LINUX)
    set(ON_LINUX ON)
elseif(CMAKE_SYSTEM_NAME MATCHES Windows)
    message(INFO "[${CMAKE_PROJECT_NAME}] - building for Windows....")
    add_definitions(-DON_WINDOWS)
    set(ON_WINDOWS ON)
elseif(CMAKE_SYSTEM_NAME MATCHES Android)
    message(INFO "[${CMAKE_PROJECT_NAME}] - building for Android....")
    add_definitions(-DON_ANDROID)
    set(ON_ANDROID ON)
elseif(CMAKE_SYSTEM_NAME MATCHES Emscripten)
    message(INFO "[${CMAKE_PROJECT_NAME}] - building for Emscripten....")
    add_definitions(-DON_EMSCRIPTEN)
    set(ON_EMSCRIPTEN ON)
else()
    message(FATAL_ERROR "system not supported")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
    add_definitions(-DIS_CLANG)
    add_compile_options(-Wall -fno-exceptions)
elseif (CMAKE_CXX_COMPILER_ID MATCHES GNU)
    add_definitions(-DIS_GCC)
    add_compile_options(-Wall -Wno-attributes -fno-exceptions)

if(CMAKE_BUILD_TYPE MATCHES Release)
    add_compile_options(-Ofast -march=native -mtune=native -flto)
endif()

elseif (CMAKE_CXX_COMPILER_ID MATCHES MSVC)
    add_definitions(-DIS_MSVC)
else()
    message(FATAL_ERROR "compiler not supported")
endif()

add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCE_DIR}/Src/Entry.cpp)

add_local_library(libos shared)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/Includes STATIC)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/Src STATIC)
add_subdirectory(${PROJECT_SOURCE_DIR}/Src)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE libos Threads::Threads)
#flagsSetup(LunaScript)
