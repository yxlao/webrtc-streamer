# project target
file(GLOB SOURCE *.cpp)
add_executable(webrtc-streamer ${SOURCE} ${WEBRTCEXTRAOBJS})
set_global_target_properties(webrtc-streamer)

add_custom_target(copy-html-dir
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/html
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/html ${CMAKE_BINARY_DIR}/html
)
add_dependencies(webrtc-streamer copy-html-dir)
