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

void sighandler(int n) {
    printf("SIGINT\n");
    // delete need thread still running
    delete webrtc_server;
    webrtc_server = nullptr;
    rtc::Thread::Current()->Quit();
}

/* ---------------------------------------------------------------------------
**  main
** -------------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
    const char* stun_url = "stun.l.google.com:19302";
    const char* webroot = "./html";
    Json::Value config;  // E.g. stores additional resource URLs.
    webrtc::AudioDeviceModule::AudioLayer audio_layer =
            webrtc::AudioDeviceModule::kPlatformDefaultAudio;

    std::string http_address("localhost:8888");
    if (argc > 1) {
        http_address = std::string(argv[1]);
    }

    std::cout << "Version:" << VERSION << std::endl;

    rtc::LogMessage::LogToDebug((rtc::LoggingSeverity)rtc::LERROR);
    rtc::LogMessage::LogTimestamps();
    rtc::LogMessage::LogThreads();
    std::cout << "Logger level:" << rtc::LogMessage::GetLogToDebug()
              << std::endl;

    rtc::Thread* thread = rtc::Thread::Current();
    rtc::InitializeSSL();

    // webrtc server
    std::list<std::string> ice_server_list;
    if ((strlen(stun_url) != 0) && (strcmp(stun_url, "-") != 0)) {
        ice_server_list.push_back(std::string("stun:") + stun_url);
    }

    webrtc_server = new PeerConnectionManager(ice_server_list, config["urls"],
                                              audio_layer, ".*", "");
    if (!webrtc_server->InitializePeerConnection()) {
        std::cout << "Cannot Initialize WebRTC server" << std::endl;
    } else {
        // http server
        std::vector<std::string> options;
        options.push_back("document_root");
        options.push_back(webroot);
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
            HttpServerRequestHandler httpServer(func, options);

            // mainloop
            signal(SIGINT, sighandler);
            thread->Run();

        } catch (const CivetException& ex) {
            std::cout << "Cannot Initialize start HTTP server exception:"
                      << ex.what() << std::endl;
        }
    }

    rtc::CleanupSSL();
    std::cout << "Exit" << std::endl;
    return 0;
}
