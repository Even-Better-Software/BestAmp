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


#define BA_CONTINUE     (0)
#define BA_ERROR_FATAL  (-2)
// #define BA_KILL         (-3)

// perhaps this is a step too far...
// this macro is useless unless it is used within the `processRequest` function
#define BA_GROW_WBUFFER(est) while (est > tcp->wbufferSize) { \
    ptrdiff_t nt = bufRealloc(&tcp->wbuffer, tcp->wbufferSize); \
    if (nt == -1) return BA_ERROR_FATAL; \
    LINFO("new buffer size %lld", nt); \
    tcp->wbufferSize = nt; }

// https://learn.microsoft.com/en-us/windows/win32/winsock/initializing-winsock

typedef struct BA_TCP_STATE {
    WSADATA wsaData;
    SOCKET  sSocket;
    SOCKET  cSocket;
    int     sts;    // stuff to send
    char*   rbuffer; size_t  rbufferSize;
    char*   wbuffer; size_t  wbufferSize;
    struct addrinfo*    result;
    struct addrinfo     hints;
} tcpState_t;


void initTcpState(tcpState_t* tcp);
void cleanupTcpState(tcpState_t* tcp);

/**
Process a request, reads from `msg`, replies will be written to `rmsg`.
    `msg` & `rmsg` must be initialized.
*/
int processRequest(appInfo_t* app, tcpState_t* tcp, 
    message_t* msg, message_t* rmsg);

/**
Windows Socket TCP Loop.
*/
int initWinsock(appInfo_t* app);

#endif

