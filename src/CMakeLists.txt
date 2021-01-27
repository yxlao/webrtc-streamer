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
    ${LIVE_SRC}
)
set_global_target_properties(webrtc-streamer)
target_include_directories(webrtc-streamer SYSTEM PRIVATE
    live/groupsock/include
    live/liveMedia/include
    live/UsageEnvironment/include
    live/BasicUsageEnvironment/include
    civetweb
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(webrtc-streamer PRIVATE
    OPENSSL_API_1_1      # civetweb
    HAVE_LIVE555         # live555helper
    NO_OPENSSL=1
    BSD=1
    SOCKLEN_T=socklen_t
    _FILE_OFFSET_BITS=64
    _LARGEFILE_SOURCE=1
)

add_custom_target(copy-html-dir
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/html
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/html ${CMAKE_BINARY_DIR}/html
)
add_dependencies(webrtc-streamer copy-html-dir)
