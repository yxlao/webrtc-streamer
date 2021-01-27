cmake_minimum_required(VERSION 3.0.0)
project(webrtc-streamer C CXX)

# WebRTC build
set(WEBRTC_ROOT "${PROJECT_SOURCE_DIR}/../webrtc" CACHE STRING "WEBRTC root directory")
set(WEBRTC_BUILD "Release" CACHE STRING "WEBRTC build type")
set(WEBRTC_DESKTOP_CAPTURE "ON" CACHE STRING "WEBRTC Desktop capture")

message(STATUS "WEBRTC_BUILD = ${WEBRTC_BUILD}")
message(STATUS "WEBRTC_ROOT = ${WEBRTC_ROOT}")
message(STATUS "WEBRTC_DESKTOP_CAPTURE = ${WEBRTC_DESKTOP_CAPTURE}")
message(STATUS "CMAKE_CXX_COMPILER_ID = ${CMAKE_CXX_COMPILER_ID}")

set(CMAKE_BUILD_TYPE ${WEBRTC_BUILD})
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

# webrtc build
if(NOT EXISTS ${WEBRTC_ROOT}/src/out/${WEBRTC_BUILD}/args.gn)
    # Example args.gn
    # is_clang=true
    # use_sysroot=false
    # is_chrome_branded=true
    # is_debug=false
    # use_custom_libcxx=false
    # rtc_include_tests=false
    # rtc_enable_protobuf=false
    # rtc_build_examples=false
    # rtc_build_tools=false
    # treat_warnings_as_errors=false
    # rtc_enable_libevent=false
    # rtc_build_libevent=false

    set(WEBRTC_ARGS
        rtc_include_tests=false\nrtc_enable_protobuf=false\nrtc_build_examples=false\nrtc_build_tools=false\ntreat_warnings_as_errors=false\nrtc_enable_libevent=false\nrtc_build_libevent=false\n
    )
    set(WEBRTC_ARGS use_custom_libcxx=false\n${WEBRTC_ARGS})
    # debug/release
    if(WEBRTC_BUILD STREQUAL "Release")
        set(WEBRTC_ARGS is_debug=false\n${WEBRTC_ARGS})
    else()
        set(WEBRTC_ARGS is_debug=true\n${WEBRTC_ARGS})
    endif()

    # enable H264 support
    set(WEBRTC_ARGS is_chrome_branded=true\n${WEBRTC_ARGS})

    # sound support
    set(WEBRTC_ARGS rtc_include_pulse_audio=false\n${WEBRTC_ARGS})
    set(WEBRTC_ARGS rtc_include_internal_audio_device=false\n${WEBRTC_ARGS})

    # compilation mode depending on target
    set(WEBRTC_ARGS use_sysroot=false\n${WEBRTC_ARGS})
    set(WEBRTC_ARGS is_clang=true\n${WEBRTC_ARGS})

    # screen capture support
    find_package(PkgConfig QUIET)
    pkg_check_modules(GTK3 QUIET gtk+-3.0)
    message("GTK_FOUND = ${GTK3_FOUND}")
    if(NOT GTK3_FOUND OR (WEBRTC_DESKTOP_CAPTURE STREQUAL "OFF"))
        set(WEBRTC_ARGS rtc_use_x11=false\nrtc_use_pipewire=false\n${WEBRTC_ARGS})
    endif()

    # write conf file
    file(WRITE ${WEBRTC_ROOT}/src/out/${WEBRTC_BUILD}/args.gn ${WEBRTC_ARGS})
endif()

set(WEBRTC_OBJS_DIR ${WEBRTC_ROOT}/src/out/${WEBRTC_BUILD}/obj)

if(NOT EXISTS ${WEBRTC_OBJS_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}webrtc${CMAKE_STATIC_LIBRARY_SUFFIX})
    execute_process(WORKING_DIRECTORY ${WEBRTC_ROOT}/src/out/${WEBRTC_BUILD}
                    COMMAND ${SHELLCOMMAND} gn gen .)
    set(NINJA_TARGET
        webrtc
        rtc_json
        jsoncpp
        builtin_video_decoder_factory
        builtin_video_encoder_factory
        peerconnection
        p2p_server_utils
        task_queue
        default_task_queue_factory
    )
    execute_process(WORKING_DIRECTORY ${WEBRTC_ROOT}/src/out/${WEBRTC_BUILD}
                    COMMAND ninja ${NINJA_TARGET})
endif()

file(GLOB_RECURSE WEBRTBBRCOBJS
    ${WEBRTC_OBJS_DIR}/modules/congestion_controller/bbr/*${CMAKE_C_OUTPUT_EXTENSION})
file(GLOB_RECURSE WEBRTJSONCPPCOBJS
     ${WEBRTC_OBJS_DIR}/third_party/jsoncpp/jsoncpp/*${CMAKE_C_OUTPUT_EXTENSION})
file(GLOB_RECURSE WEBRTP2POBJ
     ${WEBRTC_OBJS_DIR}/p2p/p2p_server_utils/*${CMAKE_C_OUTPUT_EXTENSION})
file(GLOB_RECURSE WEBRTTASK
     ${WEBRTC_OBJS_DIR}/api/task_queue/*${CMAKE_C_OUTPUT_EXTENSION}
     ${WEBRTC_OBJS_DIR}/rtc_base/rtc_task_queue_*/*${CMAKE_C_OUTPUT_EXTENSION})
set(WEBRTC_EXTRA_SRC
    ${WEBRTJSONCPPCOBJS}
    ${WEBRTBBRCOBJS}
    ${WEBRTP2POBJ}
    ${WEBRTTASK}
    ${WEBRTC_OBJS_DIR}/rtc_base/rtc_json/json${CMAKE_C_OUTPUT_EXTENSION}
)

# Dependency: WebRTC.
set(WEBRTC_INCLUDE_DIR
    ${WEBRTC_ROOT}/src
    ${WEBRTC_ROOT}/src/third_party/abseil-cpp
    ${WEBRTC_ROOT}/src/third_party/jsoncpp/source/include
    ${WEBRTC_ROOT}/src/third_party/jsoncpp/generated
    ${WEBRTC_ROOT}/src/third_party/libyuv/include
)
add_library(webrtc_extra STATIC ${WEBRTC_EXTRA_SRC})
target_include_directories(webrtc_extra SYSTEM PRIVATE ${WEBRTC_INCLUDE_DIR})
target_compile_definitions(webrtc_extra PRIVATE WEBRTC_POSIX)
target_compile_options(webrtc_extra PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
set_target_properties(webrtc_extra PROPERTIES LINKER_LANGUAGE CXX)

set(WEBRTC_LIB_DIR ${WEBRTC_OBJS_DIR})
set(WEBRTC_LIBRARIES ${CMAKE_STATIC_LIBRARY_PREFIX}webrtc${CMAKE_STATIC_LIBRARY_SUFFIX})

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
        webrtc_extra
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
    if(EXISTS ${WEBRTC_OBJS_DIR}/modules/desktop_capture/desktop_capture.ninja)
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
