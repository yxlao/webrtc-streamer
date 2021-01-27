cmake_minimum_required(VERSION 3.0.0)
project(webrtc-streamer)

set(CMAKE_BUILD_TYPE ${WEBRTCBUILD})
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

# Dependency: WebRTC.
include(webrtc.cmake)

# Dependency: Thread.
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_subdirectory(src)
