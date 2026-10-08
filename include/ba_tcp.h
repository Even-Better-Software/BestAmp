#ifndef TCP_H
#define TCP_H

#include <winsock2.h>
#include <ws2tcpip.h>

#include "ba_prefs.h"
#include "ba_common.h"


#define BA_PORT "7080"
#define BA_PORT_I (7081)
#define BA_BUFLEN (512)
#define BA_MSG_DELIM "\n"

#define BA_WSA_ERROR BA_ERROR "WSAStartup failed.\n"
#define BA_SOCKET_CREATE_ERROR BA_ERROR "socket create error.\n"
#define BA_SOCKET_BIND_ERROR BA_ERROR "socket failed to bind.\n"
#define BA_ACCEPT_FAILED_ERROR BA_ERROR "failed to accept client.\n"
#define BA_SOCKET_ERROR BA_ERROR "socket error or unexpected disconnect.\n"
#define BA_PORTAUDIO_ERROR BA_ERROR " fatal portaudio error\n"

#define BA_AWAITING_CLIENT BA_INFO \
    " waiting for client to connect.\n"
#define BA_REQUEST_RECIEVED BA_INFO \
    " request received.\n"
#define BA_REQUEST_RECEVIED_BYTES BA_INFO \
    " request received %d.\n"
#define BA_CLIENT_CONNECTED BA_INFO \
    " client connected.\n"
#define BA_CLIENT_DISCONNECT BA_INFO \
    " client disconnected.\n"
#define BA_BAD_MESSAGE_RECIEVED BA_INFO \
    " bad message received.\n"
#define BA_UPDATE_PREFS_MSG_RECEIVED BA_INFO \
    " update prefs msg.\n"
#define BA_UPDATE_AMP_VALS_MSG_RECEIVED BA_INFO \
    " update amp values msg.\n"
#define BA_QUERY_HOST_APIS_MSG_RECEIVED BA_INFO \
    " query host APIs msg.\n"
#define BA_QUERY_DEFAULT_HOST_API_MSG_RECEIVED BA_INFO \
    " query default host API msg.\n"
#define BA_QUERY_DEVICES_FOR_HOST_API_MSG_RECEIVED BA_INFO \
    " query host API devices msg.\n"

#define BA_MESSAGE_TYPE "%d"

#define BA_ERROR_FORMAT -1
#define BA_ERROR_FATAL -2
#define BA_KILL -3


// https://learn.microsoft.com/en-us/windows/win32/winsock/initializing-winsock

/**
Regarding messages
they absolutely must contain all values

strtok / strtok_s is LAME & does not support consecutive delimeters & therefore
empty CSV.

update prefs:
1,<HOSTAPIIDX>,<INDEVIDX>,<OUTDEVIDX>,<INCHANNELN>,<OUTCHANNELN>,<FRAMESPERBUFFER>,<SAMPLERATE>

update amp values:
2,<GAIN>,<BASS>,<TREBLE>,<TREBLE>

20261005
examples (w/ ncat directly transmitting the message):
    2,3,3,-1,-1
    2,6,5,6,8
    1,2,16,16,2,2,512,44100.0
    1,2,16,16,1,1,512,44100.0
*/


extern char* BA_NO_CONTENT;


typedef enum BA_MESSAGE_TYPES {
    BAD = -1,
    KILL = 0,               // stop the application
    UPDATE_PREFS = 1,
    UPDATE_AMP_VALS,
    QUERY_HOST_APIS,
    QUERY_DEFAULT_HOST_API,
    QUERY_DEVICES_FOR_HOST_API
} messageType_t;


typedef struct BA_MSG {
    messageType_t type;
    int length;         // uint32_t (fixed width)
    int data_off;       // should always be 8 bytes but for future use
    char* data;         // this is dangerous though because it wont live on the heap
                        // only for the duration of the socket function call
                        // however, that should be the duration of the entire program
                        // so this is probably ok... something to think about
} message_t;


typedef struct BA_UPDATE_PREFS_MSG {
    int hostIdx, inDevIdx, outDevIdx, inChanneln, outChanneln;
} updatePrefsMsg_t;     // not actually used (but it might be)


typedef struct BA_UPDATE_AMP_VALS_MSG {
    int gain, bass, treble, power;
} updateAmpValsMsg_t;   // not actually used (but it might be)


typedef struct BA_TCP_STATE {
    WSADATA wsaData;
    SOCKET  sSocket;
    SOCKET  cSocket;
    char*   buffer;
    struct addrinfo*    result;
    struct addrinfo     hints;
} tcpState_t;

// dont need this anymore
// messageType_t parseMessageType(char* msg);


void initTcpState(tcpState_t* tcp);

/**
Accepts a message byte array (char) & parses into the header struct.
    [WO] should return some sort of parse code
    0 -> OK
    -1 -> parse failed (probably invalid type)
*/
int parseMessage(message_t* pMsg, char* msg);

/**
For debugging purposes.
*/
void printMessage(message_t* amsg);

void parsePrefsFromMsg(baPrefs_t* prefs, message_t* msg);
void parseAmpValuesFromMsg(ampValues_t* ampValues, message_t* msg);

int processMessage(appInfo_t* app, message_t* msg);

/**
Windows Socket TCP Loop.
*/
int initWinsock(appInfo_t* app);


/**
BestAmp PortAudio callback definition.
    [WO] I don't really know where to put this
*/
int bestAmpCB(
    const void* input,
    void* output,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData 
);

#endif

