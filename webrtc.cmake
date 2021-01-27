include(ExternalProject)

ExternalProject_Add(
    ext_webrtc
    PREFIX webrtc
    URL "/home/yixing/repo/webrtc"
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_ALWAYS ON
)

set(WEBRTC_INCLUDE_DIR "") # "/" is critical.
set(WEBRTC_LIB_DIR "")
set(WEBRTC_LIBRARIES ${CMAKE_STATIC_LIBRARY_PREFIX}webrtc${CMAKE_STATIC_LIBRARY_SUFFIX})
