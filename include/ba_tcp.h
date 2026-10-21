#ifndef TCP_H
#define TCP_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <portaudio.h>

#include "ba_portaudio_helpers.h"
#include "ba_portaudio_cb.h"
#include "ba_common.h"
#include "ba_prefs.h"
#include "ba_amp.h"
#include "ba_tcp_messages.h"


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

#define BA_CONTINUE     (0)
#define BA_ERROR_FORMAT (-1)
#define BA_ERROR_FATAL  (-2)
#define BA_KILL         (-3)


// https://learn.microsoft.com/en-us/windows/win32/winsock/initializing-winsock


extern char* BA_NO_CONTENT;


typedef struct BA_TCP_STATE {
    WSADATA wsaData;
    SOCKET  sSocket;
    SOCKET  cSocket;
    int     sts;    // stuff to send
    char*   rbuffer;
    char*   wbuffer;
    struct addrinfo*    result;
    struct addrinfo     hints;
} tcpState_t;

// dont need this anymore
// messageType_t parseMessageType(char* msg);


void initTcpState(tcpState_t* tcp);
void cleanupTcpState(tcpState_t* tcp);

/**
Process a request, reads from `msg`, replies will be written to `rmsg`.
    `msg` & `rmsg` must be initialized.
*/
int processRequest(appInfo_t* app, message_t* msg, message_t* rmsg,
    char* rbuf, char* wbuf);

/**
Windows Socket TCP Loop.
*/
int initWinsock(appInfo_t* app);

#endif

