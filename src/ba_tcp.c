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


char* BA_NO_CONTENT = NULL;


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


int parseMessage(message_t* pMsg, char* msg)
{
    // Byte 0 -> message type
    //      store in `type`
    // Byte 1 -> length of the message
    //      store in `length`
    // Byte 2 -> from this point onwards, its message specific content
    //      advance the pointer by the `length` & then
    //      assign to the `data` field of the `message_t` struct

    // as per our protocol this never changes literally interpret this as an int
    int typeByte = 0;
    memcpy(&typeByte, msg, sizeof(typeByte));
    typeByte = ntohl(typeByte);
    // as per out protocol this never changes literally interpret this as an int
    int lengthByte = 0; // this keeps the thing advanced?
    memcpy(&lengthByte, (msg + sizeof(typeByte)), sizeof(lengthByte));
    lengthByte = ntohl(lengthByte);

    switch (typeByte) {
    case KILL:
        pMsg->type = KILL;
        pMsg->length = 0;
        pMsg->data = BA_NO_CONTENT; // can skip the other stuff
        return 0;
    case UPDATE_PREFS:
        pMsg->type = UPDATE_PREFS; 
        break;
    case UPDATE_AMP_VALS:
        pMsg->type = UPDATE_AMP_VALS;
        break;
    case QUERY_HOST_APIS:
        pMsg->type = QUERY_HOST_APIS;
        break;
    case QUERY_DEFAULT_HOST_API:
        pMsg->type = QUERY_DEFAULT_HOST_API;
        break;
    case QUERY_DEVICES_FOR_HOST_API:
        pMsg->type = QUERY_DEVICES_FOR_HOST_API;
        break;
    default:
        return -1;
    }
     
    if (lengthByte == 0) {
        pMsg->length = 0;
        pMsg->data = BA_NO_CONTENT; // set this explicitly (this is just NULL)
    } else if (lengthByte > 0) {
        // this many bytes should be in the message
        pMsg->length = lengthByte;
        // do I need to advance twice or just once cause I already advanced?
        pMsg->data = msg + sizeof(typeByte) + sizeof(lengthByte);
    }

    return 0;
}


void parsePrefsFromMsg(baPrefs_t* prefs, message_t* msg)
{
    // tokenize then parse
    /*
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
    */

    if (msg->length != 0x1C)
        fprintf(stderr, "\e[31update prefs content length should be 28 bytes.\e[0m\n");

    char* tmp = msg->data;

    memcpy(&prefs->hostApiIdx, tmp, sizeof(prefs->hostApiIdx));
    prefs->hostApiIdx = ntohl(prefs->hostApiIdx);

    memcpy(&prefs->inDevIdx, tmp += sizeof(prefs->hostApiIdx), sizeof(prefs->inDevIdx));
    prefs->inDevIdx = ntohl(prefs->inDevIdx);

    memcpy(&prefs->outDevIdx, tmp += sizeof(prefs->inDevIdx), sizeof(prefs->outDevIdx));
    prefs->outDevIdx = ntohl(prefs->outDevIdx);

    memcpy(&prefs->inChanneln, tmp += sizeof(prefs->outDevIdx), sizeof(prefs->inChanneln));
    prefs->inChanneln = ntohl(prefs->inChanneln);

    memcpy(&prefs->outChanneln, tmp += sizeof(prefs->inChanneln), sizeof(prefs->outChanneln));
    prefs->outChanneln = ntohl(prefs->outChanneln);

    memcpy(&prefs->framesPerBuffer, tmp += sizeof(prefs->outChanneln), sizeof(prefs->framesPerBuffer));
    prefs->framesPerBuffer = ntohl(prefs->framesPerBuffer);
   
    int tmpi;
    memcpy(&tmpi, tmp += sizeof(prefs->framesPerBuffer), sizeof(tmpi));
    tmpi = ntohl(tmpi);   // cheat to get the correct byte-ordering
    memcpy(&prefs->sampleRate, &tmpi, sizeof(prefs->sampleRate));
     
    fprintf(stdout, BA_INFO " parsed updated prefs "); 
}


void parseAmpValuesFromMsg(ampValues_t* ampValues, message_t* msg)
{
    // tokenize then parse
    /*
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
    */

    if (msg->length != 0x10)
        fprintf(stderr, "\e[31mupdate amp values content length should be 16 bytes.\e[0m\n");

    char* tmp = msg->data;

    memcpy(&ampValues->gain, tmp, sizeof(ampValues->gain));
    ampValues->gain = ntohl(ampValues->gain);

    memcpy(&ampValues->bass, tmp += sizeof(ampValues->gain), sizeof(ampValues->bass));
    ampValues->bass = ntohl(ampValues->bass);

    memcpy(&ampValues->treble, tmp += sizeof(ampValues->bass), sizeof(ampValues->treble));
    ampValues->treble = ntohl(ampValues->treble);

    memcpy(&ampValues->power, tmp += sizeof(ampValues->treble), sizeof(ampValues->power));
    ampValues->power = ntohl(ampValues->power);
 
    fprintf(stdout, BA_INFO " parsed updated amp values "); 
}


void printMessage(message_t* msg)
{
    printf("type: %d, length: %d\n", msg->type, msg->length);
}


int processMessage(appInfo_t* app, message_t* msg)
{
    PaError err;
    int status = 0;
    
    // [WO] basically copy all of the setup code here. 
    // IF Request Type is 1
    //      Parse the BA_UPDATE_PREFS_MSG -> set prefs accordingly
    //          Set using coalesce function (so defaults can be used)
    //      Setup the portaudio stream w/ the values from the request
    //      Session State -> stream running [WO] ignore this
    // IF Request Type is 2
    //      Set amp values

    switch (msg->type) {
    case BAD:
        fprintf(stdout, BA_BAD_MESSAGE_RECIEVED);
        return BA_ERROR_FORMAT;
    case UPDATE_PREFS:
        fprintf(stdout, BA_UPDATE_PREFS_MSG_RECEIVED);
        // Parse prefs from message
        baPrefs_t pTemp;
        initBaPrefs(&pTemp);
        parsePrefsFromMsg(&pTemp, msg);
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
        parseAmpValuesFromMsg(&avTemp, msg);
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
        status = 0;
        return BA_KILL;
    } 

    return status;
}


int initWinsock(appInfo_t* app) {
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
    
    // accept loop so the client disconnecting does not trigger this
    // application shutting down
    // but then we have the problem of turning this off if the app is shutdown
    // in a way that we cannot control
    // for testing this is tricky because ncat terminates the connection
    // immediately after sending the message

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
    // char buffer[BA_BUFLEN];
    // [WO] clear & allocate this buffer
    // so I know that the memory is zeroed
    char* buffer = (char*)calloc(BA_BUFLEN, sizeof(char));
    do {
        memset(buffer, 0, BA_BUFLEN);       // zero memory per iteration
        status = recv(client_sock, buffer, BA_BUFLEN, 0);
        if (status > 0) {
            printf("Bytes recevied: %d\n", status);

            // status = read_msg(app, buffer, status);

            // parse message
            message_t msg = { 0 };
            status = parseMessage(&msg, buffer);
            printMessage(&msg);
            // if the message is kill, abort the loop (dont need to process anything else)
            // if the message is bad (-1), next iteration of the loop (drop it)
            //      invalid message reply

            // if the message is complex, route to the message processor?
            //      or route all things to the message processor?
            // if the message is valid, delegate to message type handler
            //      if handler success, OK reply or handler OK reply message & data
            //      if handler fails, BAD reply or handler BAD reply
            status = processMessage(app, &msg);

            if (status == BA_ERROR_FORMAT) {
                printf(BA_BAD_MESSAGE_RECIEVED);
                continue;
            } else if (status == BA_ERROR_FATAL) {
                printf(BA_PORTAUDIO_ERROR);
                break;
            } else if (status == BA_KILL) {
                printf("Connection closing...\n");
                break;
            }

        } else if (status == 0) {
            printf("Connection closing...\n");
            break;
        } else {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(client_sock);
            WSACleanup();
            return 1;
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

    } while (1);

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

