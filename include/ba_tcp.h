#ifndef TCP_H
#define TCP_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <portaudio.h>

#include "ba_common.h"
#include "ba_portaudio_helpers.h"
#include "ba_portaudio_cb.h"
#include "ba_prefs.h"
#include "ba_amp.h"
#include "ba_tcp_messages.h"


#define BA_PORT "7080"
#define BA_PORT_I (7081)
#define BA_BUFLEN (4096)

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

