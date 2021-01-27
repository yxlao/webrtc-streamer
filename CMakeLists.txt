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

# set CMAKE_FIND_ROOT_PATH to search package in WebRTC sysroot
if(CMAKE_SYSTEM_PROCESSOR MATCHES "armv.*")
    set(CMAKE_FIND_ROOT_PATH
        ${WEBRTCROOT}/src/build/linux/debian_sid_arm-sysroot)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    set(CMAKE_FIND_ROOT_PATH
        ${WEBRTCROOT}/src/build/linux/debian_sid_arm64-sysroot)
endif()
message("CMAKE_FIND_ROOT_PATH = ${CMAKE_FIND_ROOT_PATH}")

# webrtc build
if(NOT EXISTS ${WEBRTCROOT}/src/out/${WEBRTCBUILD}/args.gn)
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

    set(WEBRTCARGS
        rtc_include_tests=false\nrtc_enable_protobuf=false\nrtc_build_examples=false\nrtc_build_tools=false\ntreat_warnings_as_errors=false\nrtc_enable_libevent=false\nrtc_build_libevent=false\n
    )
    set(WEBRTCARGS use_custom_libcxx=false\n${WEBRTCARGS})
    # debug/release
    if(WEBRTCBUILD STREQUAL "Release")
        set(WEBRTCARGS is_debug=false\n${WEBRTCARGS})
    else()
        set(WEBRTCARGS is_debug=true\n${WEBRTCARGS})
    endif()

    # enable H264 support
    set(WEBRTCARGS is_chrome_branded=true\n${WEBRTCARGS})
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "armv6.*")
        set(ENV{PATH}
            "${WEBRTCROOT}/src/third_party/llvm-build/Release+Asserts/bin:$ENV{PATH}"
        )
        file(
            READ
            ${WEBRTCROOT}/src/third_party/ffmpeg/chromium/scripts/build_ffmpeg.py
            filecontent)
        string(REPLACE "--enable-armv6t2" "--disable-armv6t2" filecontent
                       "${filecontent}")
        string(REPLACE "--enable-thumb" "--disable-thumb" filecontent
                       "${filecontent}")
        string(REPLACE "--extra-cflags=-march=armv7-a"
                       "--extra-cflags=-march=armv6" filecontent
                       "${filecontent}")
        file(
            WRITE
            ${WEBRTCROOT}/src/third_party/ffmpeg/chromium/scripts/build_ffmpeg.py
            "${filecontent}")
        execute_process(
            WORKING_DIRECTORY ${WEBRTCROOT}/src/third_party/ffmpeg
            COMMAND chromium/scripts/build_ffmpeg.py linux arm --config-only)
        execute_process(WORKING_DIRECTORY ${WEBRTCROOT}/src/third_party/ffmpeg
                        COMMAND chromium/scripts/copy_config.sh)
    endif()

    # sound support
    set(WEBRTCARGS rtc_include_pulse_audio=false\n${WEBRTCARGS})
    set(WEBRTCARGS rtc_include_internal_audio_device=false\n${WEBRTCARGS})

    # compilation mode depending on target
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "armv6.*")
        set(WEBRTCARGS
            target_cpu="arm"\narm_version=6\narm_float_abi="hard"\nenable_libaom=false\n${WEBRTCARGS}
        )
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv.*")
        set(WEBRTCARGS target_cpu="arm"\n${WEBRTCARGS})
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        set(WEBRTCARGS target_cpu="arm64"\n${WEBRTCARGS})
    endif()
    if(DEFINED CMAKE_FIND_ROOT_PATH)
        set(WEBRTCARGS use_sysroot=true\n${WEBRTCARGS})
    else()
        set(WEBRTCARGS use_sysroot=false\n${WEBRTCARGS})
    endif()
    set(WEBRTCARGS is_clang=true\n${WEBRTCARGS})

    # screen capture support
    find_package(PkgConfig QUIET)
    pkg_check_modules(GTK3 QUIET gtk+-3.0)
    message("GTK_FOUND = ${GTK3_FOUND}")
    if(NOT GTK3_FOUND OR (WEBRTCDESKTOPCAPTURE STREQUAL "OFF"))
        set(WEBRTCARGS rtc_use_x11=false\nrtc_use_pipewire=false\n${WEBRTCARGS})
    endif()

    # write conf file
    file(WRITE ${WEBRTCROOT}/src/out/${WEBRTCBUILD}/args.gn ${WEBRTCARGS})
endif()

set(WEBRTCOBJS ${WEBRTCROOT}/src/out/${WEBRTCBUILD}/obj)

if(NOT EXISTS ${WEBRTCOBJS}/${CMAKE_STATIC_LIBRARY_PREFIX}webrtc${CMAKE_STATIC_LIBRARY_SUFFIX})
    execute_process(WORKING_DIRECTORY ${WEBRTCROOT}/src/out/${WEBRTCBUILD}
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
    execute_process(WORKING_DIRECTORY ${WEBRTCROOT}/src/out/${WEBRTCBUILD}
                    COMMAND ninja ${NINJA_TARGET})
endif()

file(GLOB_RECURSE WEBRTBBRCOBJS
    ${WEBRTCOBJS}/modules/congestion_controller/bbr/*${CMAKE_C_OUTPUT_EXTENSION})
file(GLOB_RECURSE WEBRTJSONCPPCOBJS
     ${WEBRTCOBJS}/third_party/jsoncpp/jsoncpp/*${CMAKE_C_OUTPUT_EXTENSION})
file(GLOB_RECURSE WEBRTP2POBJ
     ${WEBRTCOBJS}/p2p/p2p_server_utils/*${CMAKE_C_OUTPUT_EXTENSION})
file(GLOB_RECURSE WEBRTTASK
     ${WEBRTCOBJS}/api/task_queue/*${CMAKE_C_OUTPUT_EXTENSION}
     ${WEBRTCOBJS}/rtc_base/rtc_task_queue_*/*${CMAKE_C_OUTPUT_EXTENSION})
set(WEBRTCEXTRAOBJS
    ${WEBRTJSONCPPCOBJS} ${WEBRTBBRCOBJS} ${WEBRTP2POBJ} ${WEBRTTASK}
    ${WEBRTCOBJS}/rtc_base/rtc_json/json${CMAKE_C_OUTPUT_EXTENSION})

# Dependency: WebRTC.
set(WEBRTC_INCLUDE_DIR
    ${WEBRTCROOT}/src
    ${WEBRTCROOT}/src/third_party/abseil-cpp
    ${WEBRTCROOT}/src/third_party/jsoncpp/source/include
    ${WEBRTCROOT}/src/third_party/jsoncpp/generated
    ${WEBRTCROOT}/src/third_party/libyuv/include
)
set(WEBRTC_LIB_DIR ${WEBRTCOBJS})
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

    # ARM64 specific.
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "armv6.*")
        target_link_libraries(${target} -Wl,-Bstatic -latomic -Wl,-Bdynamic)
        add_definitions(-marm -march=armv6 -mfpu=vfp -mfloat-abi=hard)
    endif()
    # Static link of stdc++ if available on arm (unresolved symbol on x86-64)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm.*")
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag("-static-libgcc" CXX_SUPPORTS_STATIC_LIBGCC)
        if(CXX_SUPPORTS_STATIC_LIBGCC)
            target_link_libraries(${target} -static-libgcc)
        endif()
        check_cxx_compiler_flag("-static-libstdc++"
                                CXX_SUPPORTS_STATIC_LIBSTDCPP)
        if(CXX_SUPPORTS_STATIC_LIBSTDCPP)
            target_link_libraries(${target} -static-libstdc++)
        endif()
    endif()

    # TODO: Fix warnings.
    # target_compile_options(${target} PRIVATE
    #     $<$<COMPILE_LANGUAGE:C>:$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX ${DISABLE_MSVC_WARNINGS}>>
    #     $<$<COMPILE_LANGUAGE:C>:$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang,Intel>:-Wall -Werror>>
    #     $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX ${DISABLE_MSVC_WARNINGS}>>
    #     $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang,Intel>:-Wall -Werror>>
    # )
endfunction()

add_subdirectory(src)
