function(set_global_target_properties target)
    add_dependencies(${target} ext_webrtc webrtc_extra)
    target_link_directories(${target} PRIVATE
        ${WEBRTC_LIB_DIR}
    )
    target_link_libraries(${target} PRIVATE
        ${WEBRTC_LIBRARIES}
        Threads::Threads
        dl
        X11         # optional: X11 (desktop capture)
        Xext        # optional: X11 (desktop capture)
        Xdamage     # optional: X11 (desktop capture)
        Xfixes      # optional: X11 (desktop capture)
        Xcomposite  # optional: X11 (desktop capture)
        Xrandr      # optional: X11 (desktop capture)
        Xtst        # optional: X11 (desktop capture)
    )
    target_compile_definitions(${target} PRIVATE
        WEBRTC_POSIX
        USE_X11     # optional: X11 (desktop capture)
    )
    target_compile_options(${target} PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    )

    # X11
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

file(GLOB WEBRTC_STREAMER_SRC *.cpp)
file(GLOB LIVE_SRC
    live/groupsock/*.c*
    live/liveMedia/*.c*
    live/UsageEnvironment/*.c*
    live/BasicUsageEnvironment/*.c*
    live555helper/*.cpp
    civetweb/civetweb.c
    civetweb/CivetServer.cpp
)

add_executable(webrtc-streamer
    ${WEBRTC_STREAMER_SRC}
    ${LIVE_SRC}                        # optional: live
)
set_global_target_properties(webrtc-streamer)
target_include_directories(webrtc-streamer SYSTEM PRIVATE
    ${WEBRTC_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    live/groupsock/include             # optional: live
    live/liveMedia/include             # optional: live
    live/UsageEnvironment/include      # optional: live
    live/BasicUsageEnvironment/include # optional: live
    civetweb                           # optional: civetweb
)
target_compile_definitions(webrtc-streamer PRIVATE
    OPENSSL_API_1_1      # optional: civetweb
    HAVE_LIVE555         # optional: live555helper
    NO_OPENSSL=1         # optional: live555helper
    SOCKLEN_T=socklen_t  # optional: live
    _FILE_OFFSET_BITS=64 # optional: civetweb
    _LARGEFILE_SOURCE=1  # optional: civetweb
)

add_custom_target(copy-html-dir
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/html
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/html ${CMAKE_BINARY_DIR}/html
)
add_dependencies(webrtc-streamer copy-html-dir)
