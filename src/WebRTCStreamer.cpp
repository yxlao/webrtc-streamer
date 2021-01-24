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

PeerConnectionManager* webRtcServer = NULL;

void sighandler(int n) {
    printf("SIGINT\n");
    // delete need thread still running
    delete webRtcServer;
    webRtcServer = NULL;
    rtc::Thread::Current()->Quit();
}

/* ---------------------------------------------------------------------------
**  main
** -------------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
    const char* turnurl = "";
    const char* defaultlocalstunurl = "0.0.0.0:3478";
    const char* defaultlocalturnurl = "turn:turn@0.0.0.0:3478";
    const char* stunurl = "stun.l.google.com:19302";
    std::string defaultWebrtcUdpPortRange = "0:65535";
    std::string localWebrtcUdpPortRange = "";
    int logLevel = rtc::LERROR;
    const char* webroot = "./html";
    std::string sslCertificate;
    webrtc::AudioDeviceModule::AudioLayer audioLayer =
            webrtc::AudioDeviceModule::kPlatformDefaultAudio;
    std::string streamName;
    std::string nbthreads;
    std::string passwdFile;
    std::string authDomain = "mydomain.com";
    std::string publishFilter(".*");
    Json::Value config;

    std::string httpAddress("localhost:8888");
    if (argc > 1) {
        httpAddress = std::string(argv[1]);
    }

    std::cout << "Version:" << VERSION << std::endl;
    std::cout << config;

    rtc::LogMessage::LogToDebug((rtc::LoggingSeverity)logLevel);
    rtc::LogMessage::LogTimestamps();
    rtc::LogMessage::LogThreads();
    std::cout << "Logger level:" << rtc::LogMessage::GetLogToDebug()
              << std::endl;

    rtc::Thread* thread = rtc::Thread::Current();
    rtc::InitializeSSL();

    // webrtc server
    std::list<std::string> iceServerList;
    if ((strlen(stunurl) != 0) && (strcmp(stunurl, "-") != 0)) {
        iceServerList.push_back(std::string("stun:") + stunurl);
    }
    if (strlen(turnurl)) {
        iceServerList.push_back(std::string("turn:") + turnurl);
    }

    webRtcServer =
            new PeerConnectionManager(iceServerList, config["urls"], audioLayer,
                                      publishFilter, localWebrtcUdpPortRange);
    if (!webRtcServer->InitializePeerConnection()) {
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
        options.push_back(httpAddress);
        options.push_back("enable_keep_alive");
        options.push_back("yes");
        options.push_back("keep_alive_timeout_ms");
        options.push_back("1000");
        if (!sslCertificate.empty()) {
            options.push_back("ssl_certificate");
            options.push_back(sslCertificate);
        }
        if (!nbthreads.empty()) {
            options.push_back("num_threads");
            options.push_back(nbthreads);
        }
        if (!passwdFile.empty()) {
            options.push_back("global_auth_file");
            options.push_back(passwdFile);
            options.push_back("authentication_domain");
            options.push_back(authDomain);
        }

        try {
            std::map<std::string, HttpServerRequestHandler::httpFunction> func =
                    webRtcServer->getHttpApi();
            std::cout << "HTTP Listen at " << httpAddress << std::endl;
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
