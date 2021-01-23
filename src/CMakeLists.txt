# project target
file(GLOB SOURCE *.cpp)
add_executable(webrtc-streamer ${SOURCE} ${WEBRTCEXTRAOBJS})
set_global_target_properties(webrtc-streamer)
