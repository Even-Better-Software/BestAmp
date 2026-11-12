#include "ba_tcp.h"


void initTcpState(tcpState_t* tcp)
{
    tcp->sSocket = INVALID_SOCKET;
    tcp->cSocket = INVALID_SOCKET;
    tcp->result = NULL;
    tcp->rbuffer = NULL;
    tcp->rbufferSize = BA_BUFLEN;
    tcp->wbuffer = NULL;
    tcp->wbufferSize = BA_BUFLEN;
    tcp->sts    = 0;

    ZeroMemory(&tcp->hints, sizeof (tcp->hints));
    tcp->hints.ai_family     = AF_INET;
    tcp->hints.ai_socktype   = SOCK_STREAM;
    tcp->hints.ai_protocol   = IPPROTO_TCP;
    tcp->hints.ai_flags      = AI_PASSIVE;
}

void cleanupTcpState(tcpState_t* tcp)
{
    if (tcp->rbuffer != NULL)
        free(tcp->rbuffer);
    if (tcp->wbuffer != NULL)
        free(tcp->wbuffer);
}

static int bufRealloc(char* buf, size_t bufSize)
{
    (void)buf;
    (void)bufSize;

    return 0;
}



int processRequest(appInfo_t* app, tcpState_t* tcp,
    message_t* msg, message_t* rmsg, char* rbuf, char* wbuf
) {
    PaError err;

    // A valid request message
    switch (msg->type) {
    case UPDATE_PREFS: {
        LINFO("%d update prefs message received", msg->type);

        // Parse prefs from message
        baPrefs_t pTemp;
        updatePrefsRep_t reply = {0};
        initBaPrefs(&pTemp);
        parsePrefsFromMsg(&pTemp, msg, rbuf);
        printBaPrefs(&pTemp);
        coalesceBaPrefsToPaDefaults(&pTemp);
        printBaPrefs(&pTemp);

        if (app->streamRunning)
            Pa_StopStream(app->stream);         // [WO] errors unhandled
        if (app->streamOpen)
            Pa_CloseStream(app->stream);        // [WO] errors unhandled

        // Create Input/Output stream parameters
        PaStreamParameters iStreamParams, oStreamParams;

        iStreamParams.device = pTemp.inDevIdx;
        iStreamParams.channelCount = pTemp.inChanneln;
        iStreamParams.suggestedLatency = pTemp.inDev->defaultLowInputLatency;
        iStreamParams.sampleFormat = pTemp.sampleFormat;
        iStreamParams.hostApiSpecificStreamInfo = NULL;

        oStreamParams.device = pTemp.outDevIdx;
        oStreamParams.channelCount = pTemp.outChanneln;
        oStreamParams.suggestedLatency = pTemp.outDev->defaultLowOutputLatency;
        oStreamParams.sampleFormat = pTemp.sampleFormat;
        oStreamParams.hostApiSpecificStreamInfo = NULL;

        // Is format supported test 
        // Pa_IsFormatSupported
        // if the format it not supported we write a bad message reply
        //      we should not write a bad message reply, rather the reply as per the spec
        // return from this method w/ BA_CONTINUE (since the server is fine, basically we are dropping this request)
        // send the update prefs bad preferences reply
        err = Pa_IsFormatSupported(&iStreamParams, &oStreamParams, pTemp.sampleRate);
        if (err != paNoError) {
            LWARN("format supported error: %s", Pa_GetErrorText(err));
            // badMessageReply(rmsg, wbuf);
            reply.code = 1;
            updatePreferencesReply(rmsg, wbuf, &reply);
            return BA_CONTINUE;
        }
        // Format supported, so pTemp becomes app->prefs
        app->baPrefs = pTemp;

        ampVoice_t cVoice = getAmpVoice(app->ampValues.voice);
        // Initial biquad calculation
        biQuadFilter_lowShelf(
            &app->ampValues.bqf_bass,
            (float)app->ampValues.bassDb,
            (float)app->baPrefs.sampleRate,
            cVoice.bassCornerFreq);
        biQuadFilter_highShelf(
            &app->ampValues.bqf_treble,
            (float)app->ampValues.trebleDb,
            (float)app->baPrefs.sampleRate,
            cVoice.trebleCornerFreq);

        // Start the portaudio stream
        err = Pa_OpenStream(  &app->stream,
                            &iStreamParams,
                            &oStreamParams,
                            app->baPrefs.sampleRate,
                            app->baPrefs.framesPerBuffer,
                            0,
                            bestAmpCB,
                            app );
        if (err != paNoError) {
            LWARN("portaudio stream open error: %s", Pa_GetErrorText(err));
            reply.code = 1;
            updatePreferencesReply(rmsg, wbuf, &reply);
            return BA_CONTINUE;
        }
        app->streamOpen = 1;
        err = Pa_StartStream(app->stream);
        if (err != paNoError) {
            LWARN("portaudio start stream error: %s", Pa_GetErrorText(err));
            reply.code = 1;
            updatePreferencesReply(rmsg, wbuf, &reply);
            return BA_CONTINUE;
        }
        LINFO("portaudio stream started");
        app->streamRunning = 1;

        // Send the update prefs OK reply
        reply.code = 0;
        updatePreferencesReply(rmsg, wbuf, &reply);
        return BA_CONTINUE;

    }
    case UPDATE_AMP_VALS: {
        LINFO("%d update amp values message recevied", msg->type);

        ampValues_t avTemp;
        initAmpValues(&avTemp);
        parseAmpValuesFromMsg(&avTemp, msg, rbuf);
        printAmpValues(&avTemp);
        ampVoice_t cVoice = getAmpVoice(avTemp.voice);
        // recalculate biquads
        biQuadFilter_lowShelf(
            &avTemp.bqf_bass,
            (float)avTemp.bassDb,
            (float)app->baPrefs.sampleRate,
            cVoice.bassCornerFreq);
        biQuadFilter_highShelf(
            &avTemp.bqf_treble,
            (float)avTemp.trebleDb,
            (float)app->baPrefs.sampleRate,
            cVoice.trebleCornerFreq);
        // because it is not possible for this to fail
        // we can send the generic OK reply
        //      actually I can envision a situation where this does fail
        //      say that a bad voice value is supplied, we should tell the
        //      client about this & then use the default voice (voice 0)
        app->ampValues = avTemp;
        genericOkReply(rmsg, wbuf);
        return BA_CONTINUE;

    }
    case QUERY_HOST_APIS: {
        LINFO("%d query host APIs message recevied", msg->type);

        queryHostApiRep_t reply = { 0 };

        // ask port audio for the host APIs
        // just pass this to the message processor basically
        // I need memory, I need N * sizeof() to store const pointers to hostapi info structs
        PaHostApiIndex hostApiCount = Pa_GetHostApiCount();
        LINFO("%d host apis", hostApiCount);
        const PaHostApiInfo** hostApis = (const PaHostApiInfo**)
            calloc(hostApiCount, sizeof(PaHostApiIndex*));
        for (int i = 0; i < hostApiCount; i++) {
            const PaHostApiInfo* hinfo = Pa_GetHostApiInfo(i);
            *(hostApis + i) = hinfo;
        }
        reply.hostApiCount = hostApiCount;
        reply.hostApis = hostApis;

        // we should probably check the buffer length here & scale it if
        //      necessary
        //      Im thinking an exponentially bounded growth
        //          (double it until some max)
        //      figure out the maximum number of host APIs
        //          (no idea what this is)
        size_t replySizeEstimate = ((hostApiCount * (44)) + 8); // 4B id + 32B name 
        LINFO("%lld message size, %lld buffer size",
            replySizeEstimate, tcp->wbufferSize);
        if (replySizeEstimate >= BA_BUFLEN) {
            LWARN("reply message overflow risk");
        } // [WO] for now, this will just whine into stderr

        queryHostApisReply(rmsg, wbuf, &reply);
        free(hostApis);

        return BA_CONTINUE;

    }
    case QUERY_DEFAULT_HOST_API: {
        LINFO("%d query default host API msg recevied", msg->type);

        int hostApiCount = 1;
        queryHostApiRep_t reply = { 0 };

        // ask port audio for the default host API
        //      if the default host API cannot be queried this is a fatal case
        PaHostApiIndex hostApiIndex = Pa_GetDefaultHostApi();
        if (hostApiIndex < 0) {
            LERROR("could not get the default host API");
            return BA_ERROR_FATAL;
        }
        
        // fprintf(stderr, "%d host apis\n", hostApiCount);
        const PaHostApiInfo** hostApis = (const PaHostApiInfo**)
            calloc(hostApiCount, sizeof(PaHostApiIndex*));
        const PaHostApiInfo* hinfo = Pa_GetHostApiInfo(hostApiIndex);
        *(hostApis + 0) = hinfo;
        reply.hostApiCount = hostApiCount;
        reply.hostApis = hostApis;

        queryHostApisReply(rmsg, wbuf, &reply);
        free(hostApis);

        return BA_CONTINUE;

    }
    case QUERY_DEVICES_FOR_HOST_API: {
        LINFO("%d query devices message recieved", msg->type);

        int hostApiIdx = 0;
        queryDeviceRep_t reply = {0};
        parseDeviceQueryFromMsg(&hostApiIdx, msg, rbuf);

        LINFO("query devices for %d host API idx", hostApiIdx);

        // Get the Host API Info Struct
        //      if this fails, then the index supplied is invalid so
        //      we send the bad reply back to the client
        const PaHostApiInfo* hostApi = Pa_GetHostApiInfo(hostApiIdx);
        if (hostApi == NULL) {
            LWARN("used supplied host API idx was bad");
            queryDevicesBadReply(rmsg, wbuf);
            return BA_CONTINUE;
        }
        reply.hostApiIdx = hostApiIdx;
        reply.deviceIdxes = (int*)
            calloc(hostApi->deviceCount, sizeof(int));
        reply.devices = (const PaDeviceInfo**)
            calloc(hostApi->deviceCount, sizeof(PaDeviceInfo*));
        reply.deviceCount = hostApi->deviceCount;
        for (int i = 0; i < hostApi->deviceCount; i++) {
            PaDeviceIndex deviceIdx     = Pa_HostApiDeviceIndexToDeviceIndex(hostApiIdx, i);
            const PaDeviceInfo* device  = Pa_GetDeviceInfo(deviceIdx);
            *(reply.deviceIdxes + i)    = deviceIdx;
            *(reply.devices + i)        = device;
        }
        // 4 32 4 4 8 = 52 bytes per device
        // we should probably check the buffer length here & scale it if
        //      necessary
        //      Im thinking an exponentially bounded growth
        //          (double it until some max)
        //      figure out the maximum number of host APIs
        //          (no idea what this is)
        size_t replySizeEstimate = ((hostApi->deviceCount * (52)) + 8);
        LINFO("%lld message size, %lld buffer size",
            replySizeEstimate, tcp->wbufferSize);
        if (replySizeEstimate >=tcp->wbufferSize) {
            LWARN("reply message overflow risk");
        } // [WO] for now, this will just whine into stderr

        queryDevicesReply(rmsg, wbuf, &reply);

        free(reply.deviceIdxes);
        free(reply.devices);

        return BA_CONTINUE;
    }
    case QUERY_AMP_VOICES: {
        LINFO("%d query amp voices message received", msg->type);

        size_t replySizeEstimate = ((getAmpVoiceCount() * (36)) + 8);
        LINFO("%lld message size, %lld buffer size",
            replySizeEstimate, tcp->wbufferSize);
        if (replySizeEstimate >= tcp->wbufferSize) {
            LWARN("reply message overflow risk");
        } // [WO] for now, this will just whine into stderr
        queryAmpVoicesReply(rmsg, wbuf);

        return BA_CONTINUE;
    }
    case KILL: {
        LINFO("%d kill message received", msg->type);
        LWARN("this message will do nothing");
        return BA_CONTINUE;

    }
    default: {
        LWARN("unknown message type received & ignored");
        return BA_CONTINUE;
    }}
}


int initWinsock(appInfo_t* app) {
    tcpState_t tcp = { 0 };
    initTcpState(&tcp);

    int exitCode = 0;

    // Initialize Winsock
    int status = WSAStartup(MAKEWORD(2,2), &tcp.wsaData); // need to manually ensure version 2.2 for some reason
    if (status != 0) {
        LERROR("WSAStartup failed: %d", status);
        return 1;
    }

    // Resolve the local address and port to be used by the server
    status = getaddrinfo(NULL, BA_PORT, &tcp.hints, &tcp.result);
    if (status != 0) {
        LERROR("getaddrinfo failed: %d", status);
        freeaddrinfo(tcp.result);
        return 1;
    }
    // Create the server socket
    tcp.sSocket = socket(tcp.result->ai_family, tcp.result->ai_socktype, tcp.result->ai_protocol);
    if (tcp.sSocket == INVALID_SOCKET) {
        LERROR("error at socket creation: %d", WSAGetLastError());
        freeaddrinfo(tcp.result);
        closesocket(tcp.sSocket);
        return 1;
    }
    // Setup the TCP listening socket
    status = bind(tcp.sSocket,
        tcp.result->ai_addr, (int)tcp.result->ai_addrlen);
    if (status == SOCKET_ERROR) {
        LERROR("error at socket bind: %d", WSAGetLastError());
        freeaddrinfo(tcp.result);
        closesocket(tcp.sSocket);
        return 1;
    }
    // addrinfo needs to be cleaned up
    freeaddrinfo(tcp.result);
    // start listening
    if (listen(tcp.sSocket, 1) == SOCKET_ERROR) { // allow only 1 connection in the backlog
        LERROR("error while opening socket for listening: %d",
            WSAGetLastError());
        closesocket(tcp.sSocket);
        return 1;
    }
    LINFO("BestAmp audio engine listening on port %d", BA_PORT_I);
    // accept a client
    tcp.cSocket = accept(tcp.sSocket, NULL, NULL);
    if (tcp.cSocket == INVALID_SOCKET) {
        LERROR("error while accepting connection on socket: %d",
            WSAGetLastError());
        closesocket(tcp.sSocket);
        return 1;
    }
    LINFO("client connected");
    // no need to keep the original socket anymore, we just want the one client socket
    closesocket(tcp.sSocket);


    // Receive until the peer shuts down the connection
    // so I know that the memory is zeroed
    tcp.rbuffer = (char*)calloc(tcp.rbufferSize, sizeof(char));
    tcp.wbuffer = (char*)calloc(tcp.wbufferSize, sizeof(char));

    message_t reqMsg = { 0 };
    message_t repMsg = { 0 };

    do {
        memset(tcp.rbuffer, 0, tcp.rbufferSize);
        memset(tcp.wbuffer, 0, tcp.wbufferSize);

        reqMsg = (message_t){ 0 };
        repMsg = (message_t){ 0 };

        // [WO] TODO safe read
        status = recv(tcp.cSocket, tcp.rbuffer, BA_BUFLEN, 0);

        if (status > 0) {
            // parse message
            //      parse returns n < 0 it fails write bad message reply
            // [WO] why does this exit the program?
            status = parseMessage(&reqMsg, tcp.rbuffer);
            if (status < 0) {
                LERROR("bad message recieved");
                badMessageReply(&repMsg, tcp.wbuffer);
                tcp.sts = 1;
                exitCode = 1;
                goto cleanup;
            }
            // process request / message (delegates to message handlers)
            // process message paths will write specific replies to `repMsg`
            //      where this is the accounting for the message
            // and into `tcp.wbuffer`
            //      where this is the data for the message
            status = processRequest(app, &tcp, &reqMsg, &repMsg,
                tcp.rbuffer, tcp.wbuffer);
            if (status == BA_ERROR_FATAL) {
                LERROR("fatal port audio error while processing request message");
                badMessageReply(&repMsg, tcp.wbuffer);
                tcp.sts = 1;
                exitCode = 1;
                goto cleanup;
            }
            printMessage(&repMsg);

            // send the reply
            //      we don't explicity check this but this is the BA_CONTINUE path
            status = send(tcp.cSocket, tcp.wbuffer,
                repMsg.length + BA_HEADER_SIZE_B, 0);
            if (status == SOCKET_ERROR) {
                LERROR("fatal error while submitting reply message");
                exitCode = 1;
                goto cleanup;
            }

            LINFO("Bytes sent: %d", status);
            tcp.sts = 0;

        } else if (status == 0) {
            LINFO("Connection closing...");
            byeMessageReply(&repMsg, tcp.wbuffer);
            tcp.sts = 1;
            break;
        } else {
            LERROR("recv on incoming message failed: %d", WSAGetLastError());
            byeMessageReply(&repMsg, tcp.wbuffer);
            tcp.sts = 1;
            exitCode = 1;
            goto cleanup;
        }

    } while (1);

cleanup:
    // final reply attempt
    if (tcp.sts == 1 && tcp.cSocket != INVALID_SOCKET) {
        int status = send(tcp.cSocket,
            tcp.wbuffer, repMsg.length + BA_HEADER_SIZE_B, 0);
        if (status == SOCKET_ERROR) {
            LERROR("final send failed: %d", status);
        } else {
            LINFO("final bytes sent: %d", status);
        }
        tcp.sts = 0;
    }

    // Portaudio specific stream shutdown
    if (app->streamRunning)
        Pa_StopStream(app->stream);
    if (app->streamOpen)
        Pa_CloseStream(app->stream);

    // Socket cleanup
    cleanupTcpState(&tcp);
    if (tcp.cSocket != INVALID_SOCKET) {
        shutdown(tcp.cSocket, SD_SEND);
        closesocket(tcp.cSocket);
    }
    WSACleanup();
    
    if (exitCode == 0) {
        LINFO("tcp server closed without error");
        return 0;
    } else {
        LINFO("tcp server closed with error");
        return 1;
    }
}

