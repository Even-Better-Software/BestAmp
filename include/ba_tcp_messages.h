#ifndef BA_TCP_MESSAGES
#define BA_TCP_MESSAGES


#include <stdio.h>
#include <string.h>
#include <winsock.h>


#include "ba_common.h"
#include "ba_prefs.h"
#include "ba_amp.h"


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


#endif
