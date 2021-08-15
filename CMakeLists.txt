cmake_minimum_required(VERSION 3.21)

project(CoroutineTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_compile_definitions(SPDLOG_FMT_EXTERNAL)

file(GLOB_RECURSE src_files CONFIGURE_DEPENDS
        "*.h"
        "*.cpp")

add_executable(CoroutineTest ${src_files})


target_precompile_headers(CoroutineTest PRIVATE "pch.h")


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS thread REQUIRED)
find_package(Boost COMPONENTS context REQUIRED)
#find_package(Boost COMPONENTS coroutine2 REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

target_link_libraries(CoroutineTest PRIVATE
    Threads::Threads
    Boost::thread
    Boost::context
    #Boost::coroutine2
    fmt::fmt
    spdlog::spdlog)

