# WebRTC-streamer

WebRTC-streamer is an experiment to stream video capture devices and RTSP sources through WebRTC using simple mechanism.

It embeds a HTTP server that implements API and serves a simple HTML page that use them through AJAX.

The WebRTC signaling is implemented through HTTP requests:

- /api/call : send offer and get answer
- /api/hangup : close a call

- /api/addIceCandidate : add a candidate
- /api/getIceCandidate : get the list of candidates

The list of HTTP API is available using /api/help.

Nowdays there is builds on [CircleCI](https://circleci.com/gh/mpromonet/webrtc-streamer), [Appveyor](https://ci.appveyor.com/project/mpromonet/webrtc-streamer), [CirrusCI](https://cirrus-ci.com/github/mpromonet/webrtc-streamer) and [GitHub CI](https://github.com/mpromonet/webrtc-streamer/actions) :

- for x86_64 on Ubuntu Bionic
- for armv7 crosscompiled (this build is running on Raspberry Pi2 and NanoPi NEO)
- for armv6+vfp crosscompiled (this build is running on Raspberry PiB and should run on a Raspberry Zero)
- Windows x64 build with clang

The webrtc stream name could be :

- an alias defined using -n argument then the corresponding -u argument will be used to create the capturer
- an "rtsp://" url that will be openned using an RTSP capturer based on live555
- an "file://" url that will be openned using an MKV capturer based on live555
- an "screen://" url that will be openned by webrtc::DesktopCapturer::CreateScreenCapturer
- an "window://" url that will be openned by webrtc::DesktopCapturer::CreateWindowCapturer
- a V4L2 capture device name

## Dependencies :

It is based on :

- [WebRTC Native Code Package](http://www.webrtc.org) for WebRTC
- [civetweb HTTP server](https://github.com/civetweb/civetweb) for HTTP server
- [live555](http://www.live555.com/liveMedia) for RTSP/MKV source

# Build

## Install the Chromium depot tools (for WebRTC).

    pushd ..
    git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
    export PATH=$PATH:`realpath depot_tools`
    popd

## Download WebRTC

    mkdir ../webrtc
    pushd ../webrtc
    fetch --no-history webrtc
    popd

## Build WebRTC Streamer

    cmake . && make

It is possible to specify cmake parameters WEBRTCBUILD & WEBRTCROOT that indicate build type and path to WebRTC SDK :

- $WEBRTCROOT/src should contains source (default is $(pwd)/../webrtc)
- $WEBRTCROOT/src/out/$WEBRTCBUILD should contains libraries (default is Release)

# Usage

    ./webrtc-streamer [-H http port] [-S[embedded stun address]] -[v[v]]  [url1]...[urln]
    ./webrtc-streamer [-H http port] [-s[external stun address]] -[v[v]] [url1]...[urln]
    ./webrtc-streamer -V
        	-v[v[v]]           : verbosity
        	-V                 : print version

        	-H [hostname:]port : HTTP server binding (default 0.0.0.0:8000)
    	-w webroot         : path to get files
    	-c sslkeycert      : path to private key and certificate for HTTPS
    	-N nbthreads       : number of threads for HTTP server
    	-A passwd          : password file for HTTP server access
    	-D authDomain      : authentication domain for HTTP server access (default:mydomain.com)

    	-S[stun_address]                   : start embedded STUN server bind to address (default 0.0.0.0:3478)
    	-s[stun_address]                   : use an external STUN server (default:stun.l.google.com:19302 , -:means no STUN)
    	-t[username:password@]turn_address : use an external TURN relay server (default:disabled)
    	-T[username:password@]turn_address : start embedded TURN server (default:disabled)

        	-a[audio layer]    : spefify audio capture layer to use (default:3)

    	-n name -u videourl -U audiourl : register a name for a video url and an audio url
         	[url]                           : url to register in the source list
    	-C config.json                  : load urls from JSON config file

Arguments of '-H' are forwarded to option [listening_ports](https://github.com/civetweb/civetweb/blob/master/docs/UserManual.md#listening_ports-8080) of civetweb, then it is possible to use the civetweb syntax like `-H8000,9000` or `-H8080r,8443s`.

## Examples

    ./webrtc-streamer rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov

[Live Demo](https://webrtc-streamer.herokuapp.com/)

[Live Demo](https://webrtc-streamer.herokuapp.com/?layout=2x4)

# Using embedded STUN/TURN server behind a NAT:

It is possible start embedded ICE server and publish its url using:

    ./webrtc-streamer -S0.0.0.0:3478 -s$(curl -s ifconfig.me):3478
    ./webrtc-streamer -s- -T0.0.0.0:3478 -tturn:turn@$(curl -s ifconfig.me):3478
    ./webrtc-streamer -S0.0.0.0:3478 -s$(curl -s ifconfig.me):3478 -T0.0.0.0:3479 -tturn:turn@$(curl -s ifconfig.me):3479

The command `curl -s ifconfig.me` is getting the public IP, it could also given as a static parameter.

In order to configure the NAT rules using the upnp feature of the router, it is possible to use [upnpc](https://manpages.debian.org/unstable/miniupnpc/upnpc.1.en.html) like this:

    upnpc -r 8000 tcp 3478 tcp 3478 udp

Adapting with the HTTP port, STUN port, TURN port.

# Embed in a HTML page:

Instead of using the internal HTTP server, it is easy to display a WebRTC stream in a HTML page served by another HTTP server. The URL of the webrtc-streamer to use should be given creating the [WebRtcStreamer](http://htmlpreview.github.io/?https://github.com/mpromonet/webrtc-streamer-html/blob/master/jsdoc/WebRtcStreamer.html) instance :

    var webRtcServer      = new WebRtcStreamer(<video tag>, <webrtc-streamer url>);

A short sample HTML page using webrtc-streamer running locally on port 8000 :

    <html>
    <head>
    <script src="libs/adapter.min.js" ></script>
    <script src="webrtcstreamer.js" ></script>
    <script>
        var webRtcServer      = null;
        window.onload         = function() {
            webRtcServer      = new WebRtcStreamer("video",location.protocol+"//"+window.location.hostname+":8000");
    	webRtcServer.connect("rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov");
        }
        window.onbeforeunload = function() { webRtcServer.disconnect(); }
    </script>
    </head>
    <body>
        <video id="video" />
    </body>
    </html>
