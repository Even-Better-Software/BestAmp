#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <portaudio.h>

#include "ba_amp.h"
#include "ba_portaudio_helpers.h"
#include "ba_tcp.h"
#include "ba_prefs.h"
#include "ba_common.h"


// BestAmp PortAudio callback implementation.
int bestAmpCB(
    const void* input,
    void* output,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData 
)
{
    const float* in = (float*)input; // this has to be derived from the chosen buffer format
    float* out = (float*)output; // this has to be derived from the chosen buffer format
    // sysInfo_t* sysInfo = (sysInfo_t*)userData;
    appInfo_t* appInfo = (appInfo_t*)userData;
    // (void)userData;
    (void)timeInfo;
    (void)statusFlags;

    // [WO] todo copy input to output
    // where 2 is the number of channels
    float y;
    for (unsigned int i = 0; i < framesPerBuffer * appInfo->baPrefs.inChanneln; i++)
    {
        y = appInfo->ampValues.gain * *in;         // pre-amplification

        y = biQuadFilter_process(
            &appInfo->ampValues.bqf_bass, y);      // execute bass biQuad filter process
        y = biQuadFilter_process(
            &appInfo->ampValues.bqf_treble, y);    // execute treble biQuad filter process

        y = appInfo->ampValues.power * y;          // power-amplification

        *out = y; // write the processed sample to the output buffer
        
        // advance pointers 
        out++;
        in++;
    }

    return paContinue;
}


messageType_t parseMessageType(char* msg) 
{
    int d = -1;
    int r = sscanf_s(msg, BA_MESSAGE_TYPE, &d);
    if (r == 0 || r == EOF)
        return BAD;
    switch (d) {
    case KILL:
        return KILL;
    case UPDATE_PREFS:
        return UPDATE_PREFS; 
    case UPDATE_AMP_VALS:
        return UPDATE_AMP_VALS;
    default:
        return BAD;
    }
}

void parsePrefsFromMsg(baPrefs_t* prefs, char* msg)
{
    // tokenize then parse
    char* next_token = NULL;
    char* token = strtok_s(msg, ",", &next_token);
    if (!token) return;
    token = strtok_s(NULL, ",", &next_token);
    if (token && strlen(token) > 0) prefs->hostApiIdx = atoi(token);
    token = strtok_s(NULL, ",", &next_token);
    if (token && strlen(token) > 0) prefs->inDevIdx = atoi(token);
    token = strtok_s(NULL, ",", &next_token);
    if (token && strlen(token) > 0) prefs->outDevIdx = atoi(token);
    token = strtok_s(NULL, ",", &next_token);
    if (token && strlen(token) > 0) prefs->inChanneln = atoi(token);
    token = strtok_s(NULL, ",", &next_token);
    if (token && strlen(token) > 0) prefs->outChanneln = atoi(token);
    token = strtok_s(NULL, ",", &next_token);
    if (token && strlen(token) > 0) prefs->framesPerBuffer = atoi(token);
    token = strtok_s(NULL, ",", &next_token);
    if (token && strlen(token) > 0) prefs->sampleRate = (float)atof(token);
    
    fprintf(stdout, BA_INFO " parsed updated prefs "); 
    printBaPrefs(prefs);
}

void parseAmpValuesFromMsg(ampValues_t* ampValues, char* msg)
{
    // tokenize then parse
    char* next_token = NULL;
    char* token = strtok_s(msg, ",", &next_token);
    if (!token) return;
    token = strtok_s(NULL, ",", &next_token);
    if (token && strlen(token) > 0) ampValues->gain = atoi(token);
    token = strtok_s(NULL, ",", &next_token);
    if (token && strlen(token) > 0) ampValues->bass = atoi(token);
    token = strtok_s(NULL, ",", &next_token);
    if (token && strlen(token) > 0) ampValues->treble = atoi(token);
    token = strtok_s(NULL, ",", &next_token);
    if (token && strlen(token) > 0) ampValues->power = atoi(token);
    
    fprintf(stdout, BA_INFO " parsed updated amp values "); 
    printAmpValues(ampValues);
}


int initWinsock(appInfo_t* app) {
    PaError e;

    // Initialize Winsock
    WSADATA wsaData;
    int status = WSAStartup(MAKEWORD(2,2), &wsaData); // need to manually ensure version 2.2 for some reason
    if (status != 0) {
        printf("WSAStartup failed: %d\n", status);
        return 1;
    }

    struct addrinfo *result = NULL; 
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    status = getaddrinfo(NULL, BA_PORT, &hints, &result);
    if (status != 0) {
        printf("getaddrinfo failed: %d\n", status);
        WSACleanup();
        return 1;
    }

    SOCKET sock = INVALID_SOCKET;
    sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock == INVALID_SOCKET) {
        printf("Error at socket(): %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    status = bind(sock, result->ai_addr, (int)result->ai_addrlen);
    if (status == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // addrinfo needs to be cleaned up
    freeaddrinfo(result);

    // start listening
    if (listen(sock, 1) == SOCKET_ERROR) { // allow only 1 connection in the backlog
        printf("Listen failed with error: %d\n", WSAGetLastError() );
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    printf("Listening on port %s...\n", BA_PORT);

    // accept a client
    SOCKET client_sock;
    client_sock = accept(sock, NULL, NULL);
    if (client_sock == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    printf("Client connected.\n");

    // no need to keep the original socket anymore, we just want the one client socket
    closesocket(sock);

    // Receive until the peer shuts down the connection

    char buffer[BA_BUFLEN];
    do {
        status = recv(client_sock, buffer, BA_BUFLEN, 0);
        if (status > 0) {
            printf("Bytes received: %d\n", status);

            // [WO] basically copy all of the setup code here. 
            // IF Request Type is 1
            //      Parse the BA_UPDATE_PREFS_MSG -> set prefs accordingly
            //          Set using coalesce function (so defaults can be used)
            //      Setup the portaudio stream w/ the values from the request
            //      Session State -> stream running [WO] ignore this
            // IF Request Type is 2
            //      Set amp values
            messageType_t t = parseMessageType(buffer);
            switch (t) {
            case BAD:
                fprintf(stdout, BA_BAD_MESSAGE_RECIEVED);
                continue;
            case UPDATE_PREFS:
                fprintf(stdout, BA_UPDATE_PREFS_MSG_RECEIVED);
                // Parse prefs from message
                baPrefs_t pTemp;
                initBaPrefs(&pTemp);
                parsePrefsFromMsg(&pTemp, buffer);
                coalesceBaPrefsToPaDefaults(&pTemp);

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
                e = Pa_IsFormatSupported(&iStreamParams, &oStreamParams, pTemp.sampleRate);
                if (e != paNoError) {
                    printf("\e[31mFormat supported error\e[0m: %s\n", Pa_GetErrorText(e));
                    continue;
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

                e = Pa_OpenStream(  &app->stream,
                                    &iStreamParams,
                                    &oStreamParams,
                                    app->baPrefs.sampleRate,
                                    app->baPrefs.framesPerBuffer,
                                    0,
                                    bestAmpCB,
                                    app );
                if (e != paNoError) {
                    printf("\e[31mOpen stream error\e[0m: %s\n", Pa_GetErrorText(e));
                    continue;
                }
                app->streamOpen = 1;
                e = Pa_StartStream(app->stream);
                if (e != paNoError) {
                    printf("\e[31mStart stream error\e[0m: %s\n", Pa_GetErrorText(e));
                    continue;
                }
                app->streamRunning = 1;
                break;
            case UPDATE_AMP_VALS:
                fprintf(stdout, BA_UPDATE_AMP_VALS_MSG_RECEIVED);
                ampValues_t avTemp;
                initAmpValues(&avTemp);
                parseAmpValuesFromMsg(&avTemp, buffer);
                printAmpValues(&avTemp);

                // recalculate biquads
                biQuadFilter_lowShelf(
                    &avTemp.bqf_bass,
                    (float)avTemp.bass, app->baPrefs.sampleRate);
                biQuadFilter_highShelf(
                    &avTemp.bqf_treble,
                    (float)avTemp.treble, app->baPrefs.sampleRate);
                
                // [WO] Should probably check that the stuff here is -1
                // so keep old value if new value = -1
                app->ampValues = avTemp;

                break;
            case KILL:
                status = 0;
                break;
            }
            // placeholder action after receiving data (echoing it back)
            status = send(client_sock, "OK\n", 3, 0);
            if (status == SOCKET_ERROR) {
                printf("send failed: %d\n", WSAGetLastError());
                closesocket(client_sock);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", status);
        } else if (status == 0)
            printf("Connection closing...\n");
        else {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(client_sock);
            WSACleanup();
            return 1;
        }

    } while (status > 0);

    // Portaudio specific stream shutdown
    if (app->streamRunning)
        Pa_StopStream(app->stream);
    if (app->streamOpen)
        Pa_CloseStream(app->stream);

    // final shutdown and cleanup
    status = shutdown(client_sock, SD_SEND); // shutdown can fail but there's nothing to do
    closesocket(client_sock);
    WSACleanup();

    printf("TCP server closed cleanly.\n");

    return 0;
}

