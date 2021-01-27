include(ExternalProject)


# Prepare ${WEBRTCROOT}/src/out/${WEBRTCBUILD}/args.gn
# Exports: WEBRTCARGS
set(WEBRTCBUILD "Release" CACHE STRING "WEBRTC build type")
set(WEBRTCDESKTOPCAPTURE "ON" CACHE STRING "WEBRTC Desktop capture")

set(WEBRTCARGS rtc_include_tests=false\n)
set(WEBRTCARGS rtc_enable_protobuf=false\n${WEBRTCARGS})
set(WEBRTCARGS rtc_build_examples=false\n${WEBRTCARGS})
set(WEBRTCARGS rtc_build_tools=false\n${WEBRTCARGS})
set(WEBRTCARGS treat_warnings_as_errors=false\n${WEBRTCARGS})
set(WEBRTCARGS rtc_enable_libevent=false\n${WEBRTCARGS})
set(WEBRTCARGS rtc_build_libevent=false\n${WEBRTCARGS})
set(WEBRTCARGS use_custom_libcxx=false\n${WEBRTCARGS})

# Debug/Release
if(WEBRTCBUILD STREQUAL "Release")
    set(WEBRTCARGS is_debug=false\n${WEBRTCARGS})
else()
    set(WEBRTCARGS is_debug=true\n${WEBRTCARGS})
endif()
# H264 support
set(WEBRTCARGS is_chrome_branded=true\n${WEBRTCARGS})
# Sound support
set(WEBRTCARGS rtc_include_pulse_audio=false\n${WEBRTCARGS})
set(WEBRTCARGS rtc_include_internal_audio_device=false\n${WEBRTCARGS})
# Compilation mode depending on target
set(WEBRTCARGS use_sysroot=false\n${WEBRTCARGS})
set(WEBRTCARGS is_clang=true\n${WEBRTCARGS})
# Screen capture support
find_package(PkgConfig QUIET)
pkg_check_modules(GTK3 QUIET gtk+-3.0)
message("GTK_FOUND = ${GTK3_FOUND}")
if(NOT GTK3_FOUND OR (WEBRTCDESKTOPCAPTURE STREQUAL "OFF"))
    set(WEBRTCARGS rtc_use_x11=false\nrtc_use_pipewire=false\n${WEBRTCARGS})
endif()
# Write to build/args.gn for future use
file(WRITE ${CMAKE_BINARY_DIR}/args.gn ${WEBRTCARGS})
# message(FATAL_ERROR "WEBRTCARGS: ${WEBRTCARGS}")

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

ExternalProject_Add(
    ext_webrtc
    PREFIX webrtc
    DOWNLOAD_COMMAND rm -rf ext_webrtc
    COMMAND cp -ar /home/yixing/repo/webrtc ext_webrtc
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/args.gn
        <SOURCE_DIR>/src/out/${WEBRTCBUILD}/args.gn
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_ALWAYS ON
    BUILD_IN_SOURCE ON
)

ExternalProject_Add_Step(ext_webrtc build_obj
    COMMAND export PATH=$PATH:/home/yixing/repo/depot_tools
    COMMAND /home/yixing/repo/depot_tools/gn gen .
    COMMAND /home/yixing/repo/depot_tools/ninja ${NINJA_TARGET}
    WORKING_DIRECTORY <SOURCE_DIR>/src/out/${WEBRTCBUILD}
    DEPENDEES build
    DEPENDERS install
)

set(WEBRTC_INCLUDE_DIR "") # "/" is critical.
set(WEBRTC_LIB_DIR "")
set(WEBRTC_LIBRARIES ${CMAKE_STATIC_LIBRARY_PREFIX}webrtc${CMAKE_STATIC_LIBRARY_SUFFIX})
