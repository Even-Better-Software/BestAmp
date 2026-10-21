#include "ba_tcp.h"


char* BA_NO_CONTENT = NULL;


void initTcpState(tcpState_t* tcp)
{
    tcp->sSocket = INVALID_SOCKET;
    tcp->cSocket = INVALID_SOCKET;
    tcp->result = NULL;
    tcp->rbuffer = NULL;
    tcp->wbuffer = NULL;
    tcp->fatal = 0;
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



int processRequest(appInfo_t* app,
    message_t* msg, message_t* rmsg, char* rbuf, char* wbuf
)
{
    PaError err;
    int status = 0;

    (void)rmsg;
    (void)wbuf;

    // A valid request message
    if (msg->type >= KILL && msg->type <= QUERY_DEVICES_FOR_HOST_API) {
        switch (msg->type) {
        case UPDATE_PREFS:
            // [WO] longterm -> move these to message handler functions
            fprintf(stdout, BA_UPDATE_PREFS_MSG_RECEIVED);
            // Parse prefs from message
            baPrefs_t pTemp;
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
            err = Pa_IsFormatSupported(&iStreamParams, &oStreamParams, pTemp.sampleRate);
            if (err != paNoError) {
                printf("\e[31mFormat supported error\e[0m: %s\n", Pa_GetErrorText(err));
                return BA_ERROR_FORMAT;
            }
            // Format supported, so pTemp becomes app->prefs
            app->baPrefs = pTemp;
            
            // Initial biquad calculation
            biQuadFilter_lowShelf(
                &app->ampValues.bqf_bass,
                (float)app->ampValues.bass, pTemp.sampleRate);
            biQuadFilter_highShelf(
                &app->ampValues.bqf_treble,
                (float)app->ampValues.treble, pTemp.sampleRate);

            err = Pa_OpenStream(  &app->stream,
                                &iStreamParams,
                                &oStreamParams,
                                app->baPrefs.sampleRate,
                                app->baPrefs.framesPerBuffer,
                                0,
                                bestAmpCB,
                                app );
            if (err != paNoError) {
                printf("\e[31mOpen stream error\e[0m: %s\n", Pa_GetErrorText(err));
                return BA_ERROR_FATAL;
            }
            app->streamOpen = 1;
            err = Pa_StartStream(app->stream);
            if (err != paNoError) {
                printf("\e[31mStart stream error\e[0m: %s\n", Pa_GetErrorText(err));
                return BA_ERROR_FATAL;
            }
            app->streamRunning = 1;
            break;
        case UPDATE_AMP_VALS:
            fprintf(stdout, BA_UPDATE_AMP_VALS_MSG_RECEIVED);
            ampValues_t avTemp;
            initAmpValues(&avTemp);
            parseAmpValuesFromMsg(&avTemp, msg, rbuf);
            printAmpValues(&avTemp);

            // recalculate biquads
            biQuadFilter_lowShelf(
                &avTemp.bqf_bass,
                (float)avTemp.bass, app->baPrefs.sampleRate);
            biQuadFilter_highShelf(
                &avTemp.bqf_treble,
                (float)avTemp.treble, app->baPrefs.sampleRate);
            
            app->ampValues = avTemp;

            break;
        case QUERY_HOST_APIS:
            fprintf(stdout, BA_QUERY_HOST_APIS_MSG_RECEIVED);
            fprintf(stdout, BA_UNIMPLEMENTED);
            break;
        case QUERY_DEFAULT_HOST_API:
            fprintf(stdout, BA_QUERY_DEFAULT_HOST_API_MSG_RECEIVED);
            fprintf(stdout, BA_UNIMPLEMENTED);
            break;
        case QUERY_DEVICES_FOR_HOST_API:
            fprintf(stdout, BA_QUERY_DEVICES_FOR_HOST_API_MSG_RECEIVED);
            fprintf(stdout, BA_UNIMPLEMENTED);
            break;
        case KILL:
            break;
        } 

    // Not a request message
    } else {
        fprintf(stderr, "cannot process a message that is not a request (message ID out of request range).\n");
        status = -1;
    }

    return status;
}


int initWinsock(appInfo_t* app) {
    tcpState_t tcp = { 0 };
    initTcpState(&tcp);

    int exitCode = 0;

    // Initialize Winsock
    int status = WSAStartup(MAKEWORD(2,2), &tcp.wsaData); // need to manually ensure version 2.2 for some reason
    if (status != 0) {
        printf("WSAStartup failed: %d\n", status);
        return 1;
    }

    // Resolve the local address and port to be used by the server
    status = getaddrinfo(NULL, BA_PORT, &tcp.hints, &tcp.result);
    if (status != 0) {
        printf("getaddrinfo failed: %d\n", status);
        freeaddrinfo(tcp.result);
        return 1;
    }
    // Create the server socket
    tcp.sSocket = socket(tcp.result->ai_family, tcp.result->ai_socktype, tcp.result->ai_protocol);
    if (tcp.sSocket == INVALID_SOCKET) {
        printf("Error at socket(): %d\n", WSAGetLastError());
        freeaddrinfo(tcp.result);
        closesocket(tcp.sSocket);
        return 1;
    }
    // Setup the TCP listening socket
    status = bind(tcp.sSocket,
        tcp.result->ai_addr, (int)tcp.result->ai_addrlen);
    if (status == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(tcp.result);
        closesocket(tcp.sSocket);
        return 1;
    }
    // addrinfo needs to be cleaned up
    freeaddrinfo(tcp.result);
    // start listening
    if (listen(tcp.sSocket, 1) == SOCKET_ERROR) { // allow only 1 connection in the backlog
        printf("Listen failed with error: %d\n", WSAGetLastError() );
        closesocket(tcp.sSocket);
        return 1;
    }
    printf("Listening on port %s...\n", BA_PORT);
    // accept a client
    tcp.cSocket = accept(tcp.sSocket, NULL, NULL);
    if (tcp.cSocket == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(tcp.sSocket);
        return 1;
    }
    printf("Client connected.\n");
    // no need to keep the original socket anymore, we just want the one client socket
    closesocket(tcp.sSocket);

    // Receive until the peer shuts down the connection
    // [WO] clear & allocate this buffer
    // so I know that the memory is zeroed
    tcp.rbuffer = (char*)calloc(BA_BUFLEN, sizeof(char));
    tcp.wbuffer = (char*)calloc(BA_BUFLEN, sizeof(char));

    message_t reqMsg = { 0 };
    message_t repMsg = { 0 };

    do {
        memset(tcp.rbuffer, 0, BA_BUFLEN);
        memset(tcp.wbuffer, 0, BA_BUFLEN);

        reqMsg = (message_t){ 0 };
        repMsg = (message_t){ 0 };

        // [WO] TODO safe read
        status = recv(tcp.cSocket, tcp.rbuffer, BA_BUFLEN, 0);

        if (status > 0) {
            // parse message
            status = parseMessage(&reqMsg, tcp.rbuffer);
            // parse returns n < 0 it fails write bad message reply
            if (status < 0) {
                printf(BA_BAD_MESSAGE_RECIEVED);
                badMessageReply(&repMsg, tcp.wbuffer);
                tcp.sts = 1;
                goto cleanup;
            }
            // DEBUGGING see the request (message info, not contents per se)
            printMessage(&reqMsg);

            // process request / message (delegates to message handlers)
            status = processRequest(app, &reqMsg, &repMsg,
                tcp.rbuffer, tcp.wbuffer);
            // [WO] testing, all replies for now are OK
            genericOkReply(&repMsg, tcp.wbuffer);
            printMessage(&repMsg);
            if (status == BA_ERROR_FORMAT) {
                fprintf(stderr, BA_BAD_MESSAGE_RECIEVED);
                badMessageReply(&repMsg, tcp.wbuffer);
            } else if (status == BA_ERROR_FATAL) {
                fprintf(stderr, BA_PORTAUDIO_ERROR);
                badMessageReply(&repMsg, tcp.wbuffer);
                tcp.sts = 1;
                exitCode = 1;
                goto cleanup;
            }

            // send the reply
            status = send(tcp.cSocket, tcp.wbuffer,
                repMsg.length + BA_HEADER_SIZE_B, 0);
            if (status == SOCKET_ERROR) {
                printf("send failed: %d\n", WSAGetLastError());
                goto cleanup;
            }
            printf("Bytes sent: %d\n", status);
            tcp.sts = 0;

        } else if (status == 0) {
            printf("Connection closing...\n");
            byeMessageReply(&repMsg, tcp.wbuffer);
            tcp.sts = 1;
            break;
        } else {
            printf("recv failed: %d\n", WSAGetLastError());
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
            fprintf(stderr, "final send failed: %d\n", status);
        } else {
            fprintf(stderr, "final bytes sent: %d\n", status);
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
        fprintf(stderr, "tcp server closed without error");
        return 0;
    } else {
        fprintf(stderr, "tcp server closed with error");
        return 1;
    }
}

