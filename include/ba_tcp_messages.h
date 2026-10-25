#ifndef BA_TCP_MESSAGES
#define BA_TCP_MESSAGES


#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#include "ba_common.h"
#include "ba_prefs.h"
#include "ba_amp.h"
#include "portaudio.h"


#define BA_HEADER_SIZE_B        (8)
#define BA_NUMERIC_FIELD_B      (4)
#define BA_DOUBLE_FIELD_B       (8)
#define BA_SMALL_STR_FIELD_B    (32)

#define BA_WRITE_BUFFER_NULL \
    "failed to write reply, write buffer is null\n"


typedef enum BA_MESSAGE_TYPES {
    KILL            = 0,        // stop the application
    UPDATE_PREFS,
    UPDATE_AMP_VALS,            // presumably if no value suppiled, auto increment from the last supplied value
    QUERY_HOST_APIS,
    QUERY_DEFAULT_HOST_API,
    QUERY_DEVICES_FOR_HOST_API,

    UPDATE_PREFS_REPLY  = 1001,
    UPDATE_AMP_VALS_REPLY,
    QUERY_HOST_APIS_REPLY,
    QUERY_DEFAULT_HOST_API_REPLY,
    QUERY_DEVICES_REPLY,

    BYE     = 9997,
    OK      = 9998,
    BAD     = 9999,
} messageType_t;


/*[WO] can I make GCC happy?*/
typedef enum BA_REQ_MESSAGE_TYPES {
        REQ_KILL                        = KILL,
        REQ_UPDATE_PREFS                = UPDATE_PREFS,
        REQ_UPDATE_AMP_VALS             = UPDATE_AMP_VALS,
        REQ_QUERY_HOST_APIS             = QUERY_HOST_APIS,
        REQ_QUERY_DEFAULT_HOST_API      = QUERY_DEFAULT_HOST_API,
        REQ_QUERY_DEVICES_FOR_HOST_API  = QUERY_DEVICES_FOR_HOST_API
} reqMessageType_t;


typedef struct BA_MSG {
    messageType_t type;
    int length;         // uint32_t (fixed width)
    int data_off;       // should always be 8 bytes but for future use
} message_t;


typedef struct BA_UPDATE_PREFS_REP {
    int code;           // 0 == OK, 1 == BAD preferences
} updatePrefsRep_t;


typedef struct BA_QUERY_HOST_API_REP {
    int hostApiCount;
    const PaHostApiInfo** hostApis;
} queryHostApiRep_t;


typedef struct BA_QUERY_DEVICE_API_REP {
    int hostApiIdx;
    int deviceCount;
    int* deviceIdxes;
    const PaDeviceInfo** devices;
} queryDeviceRep_t;


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
void printMessage(message_t* msg);


void parsePrefsFromMsg(baPrefs_t* prefs, message_t* msg, char* rbuf);
void parseAmpValuesFromMsg(ampValues_t* ampValues, message_t* msg, char* rbuf);
void parseDeviceQueryFromMsg(int* hostApi, message_t* msg, char* rbuf);


/**
Writes bad message accounting into `rpmsg`
Writes bad message into `rpmsg`
*/
void badMessageReply(message_t* rpmsg, char* wbuf);

/**
Writes bye message accounting into `rpmsg`
Writes bye message into `wbuf`
*/
void byeMessageReply(message_t* rpmsg, char* wbuf);

/**
Writes ok message accounting into `rpmsg`
Writes ok message into `wbuf`
*/
void genericOkReply(message_t* rpmsg, char* wbuf);


void updatePreferencesReply(message_t* rpmsg, char* wbuf, updatePrefsRep_t* content);
void updateAmpValuesReply(message_t* rpmsg, char* wbuf);


void queryHostApisReply(message_t* rpmsg, char* wbuf, queryHostApiRep_t* content);
void queryDevicesReply(message_t* rpmsg, char* wbuf, queryDeviceRep_t* content);
void queryDevicesBadReply(message_t* rpmsg, char* wbuf);

#endif

