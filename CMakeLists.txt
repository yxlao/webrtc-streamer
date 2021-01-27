cmake_minimum_required(VERSION 3.0.0)
project(webrtc-streamer)

# WebRTC build
set(WEBRTCROOT "${PROJECT_SOURCE_DIR}/../webrtc" CACHE STRING "WEBRTC root directory")
set(WEBRTCBUILD "Release" CACHE STRING "WEBRTC build type")
set(WEBRTCDESKTOPCAPTURE "ON" CACHE STRING "WEBRTC Desktop capture")

message(STATUS "WEBRTCBUILD = ${WEBRTCBUILD}")
message(STATUS "WEBRTCROOT = ${WEBRTCROOT}")
message(STATUS "WEBRTCDESKTOPCAPTURE = ${WEBRTCDESKTOPCAPTURE}")
message(STATUS "CMAKE_CXX_COMPILER_ID = ${CMAKE_CXX_COMPILER_ID}")

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

function(set_global_target_properties target)

    target_include_directories(${target} SYSTEM PRIVATE
        ${WEBRTC_INCLUDE_DIR}
    )
    target_link_directories(${target} PRIVATE
        ${WEBRTC_LIB_DIR}
    )
    target_link_libraries(${target} PRIVATE
        ${WEBRTC_LIBRARIES}
        Threads::Threads
        dl
    )
    target_compile_definitions(${target} PRIVATE
        WEBRTC_POSIX
    )
    target_compile_options(${target} PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    )

    # X11
    if(EXISTS ${WEBRTCOBJS}/modules/desktop_capture/desktop_capture.ninja)
        target_compile_definitions(${target} PRIVATE USE_X11)
        target_link_libraries(${target} PRIVATE
            X11
            Xext
            Xdamage
            Xfixes
            Xcomposite
            Xrandr
            Xtst
        )
    endif()

    set_target_properties(${target}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

    target_compile_options(${target} PRIVATE
        $<$<COMPILE_LANGUAGE:C>:$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX ${DISABLE_MSVC_WARNINGS}>>
        $<$<COMPILE_LANGUAGE:C>:$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang,Intel>:-Wall -Werror>>
        $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX ${DISABLE_MSVC_WARNINGS}>>
        $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang,Intel>:-Wall -Werror>>
    )
endfunction()

add_subdirectory(src)
