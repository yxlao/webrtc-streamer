/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** main.cpp
**
** -------------------------------------------------------------------------*/

#include <signal.h>

#include <iostream>
#include <fstream>

#include "rtc_base/ssl_adapter.h"
#include "rtc_base/thread.h"
#include "p2p/base/stun_server.h"
#include "p2p/base/basic_packet_socket_factory.h"
#include "p2p/base/turn_server.h"

#include "PeerConnectionManager.h"
#include "HttpServerRequestHandler.h"

PeerConnectionManager* webrtc_server = nullptr;

void SignalHandler(int n) {
    printf("SIGINT\n");
    // delete need thread still running
    delete webrtc_server;
    webrtc_server = nullptr;
    rtc::Thread::Current()->Quit();
}

int main(int argc, char* argv[]) {
    // Configs.
    const std::string web_root = "./html";
    const std::vector<std::string> stun_urls{"stun:stun.l.google.com:19302"};
    std::string http_address("localhost:8888");
    if (argc > 1) {
        http_address = std::string(argv[1]);
    }

    // Logging settings.
    std::cout << "Version:" << VERSION << std::endl;
    rtc::LogMessage::LogToDebug((rtc::LoggingSeverity)rtc::LERROR);
    std::cout << "Logger level:" << rtc::LogMessage::GetLogToDebug()
              << std::endl;
    rtc::LogMessage::LogTimestamps();
    rtc::LogMessage::LogThreads();

    // WebRTC server.
    rtc::Thread* thread = rtc::Thread::Current();
    rtc::InitializeSSL();
    std::list<std::string> ice_servers(stun_urls.begin(), stun_urls.end());
    Json::Value config;
    webrtc::AudioDeviceModule::AudioLayer audio_layer =
            webrtc::AudioDeviceModule::kPlatformDefaultAudio;
    webrtc_server = new PeerConnectionManager(ice_servers, config["urls"],
                                              audio_layer, ".*", "");
    if (webrtc_server->InitializePeerConnection()) {
        std::cout << "InitializePeerConnection() succeeded." << std::endl;
    } else {
        throw std::runtime_error("InitializePeerConnection() failed.");
    }

    // CivetWeb http server.
    std::vector<std::string> options;
    options.push_back("document_root");
    options.push_back(web_root);
    options.push_back("enable_directory_listing");
    options.push_back("no");
    options.push_back("additional_header");
    options.push_back("X-Frame-Options: SAMEORIGIN");
    options.push_back("access_control_allow_origin");
    options.push_back("*");
    options.push_back("listening_ports");
    options.push_back(http_address);
    options.push_back("enable_keep_alive");
    options.push_back("yes");
    options.push_back("keep_alive_timeout_ms");
    options.push_back("1000");
    try {
        std::map<std::string, HttpServerRequestHandler::httpFunction> func =
                webrtc_server->getHttpApi();
        std::cout << "HTTP Listen at " << http_address << std::endl;
        HttpServerRequestHandler civet_server(func, options);

        // Main loop.
        signal(SIGINT, SignalHandler);
        thread->Run();
    } catch (const CivetException& ex) {
        std::cout << "Cannot Initialize start HTTP server exception:"
                  << ex.what() << std::endl;
    }

    rtc::CleanupSSL();
    std::cout << "Exit" << std::endl;
    return 0;
}
