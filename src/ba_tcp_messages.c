#include "ba_tcp_messages.h"


void printMessage(message_t* msg)
{
    printf("type: %d, length: %d\n", msg->type, msg->length);
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
    int lengthByte = 0;
    memcpy(&lengthByte, (msg + sizeof(typeByte)), sizeof(lengthByte));
    lengthByte = ntohl(lengthByte);

    switch (typeByte) {
    case KILL:
        pMsg->type = KILL;
        pMsg->length = 0;
        pMsg->data = NULL;
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
        pMsg->data = NULL;
    } else if (lengthByte > 0) {
        // this many bytes should be in the message
        pMsg->length = lengthByte;
        // do I need to advance twice or just once cause I already advanced?
        pMsg->data = msg;
        pMsg->data_off = sizeof(typeByte) + sizeof(lengthByte);
    }

    return 0;
}


void parsePrefsFromMsg(baPrefs_t* prefs, message_t* msg)
{
    if (msg->length != 0x1C)
        fprintf(stderr, "\e[31update prefs content length should be 28 bytes.\e[0m\n");

    int t_data_off = msg->data_off;

    memcpy(&prefs->hostApiIdx, (
        msg->data + t_data_off
    ), sizeof(prefs->hostApiIdx));
    prefs->hostApiIdx = ntohl(prefs->hostApiIdx);

    memcpy(&prefs->inDevIdx, (
        msg->data + (t_data_off += sizeof(prefs->hostApiIdx))
    ), sizeof(prefs->inDevIdx));
    prefs->inDevIdx = ntohl(prefs->inDevIdx);

    memcpy(&prefs->outDevIdx, (
        msg->data + (t_data_off += sizeof(prefs->inDevIdx))
    ), sizeof(prefs->outDevIdx));
    prefs->outDevIdx = ntohl(prefs->outDevIdx);

    memcpy(&prefs->inChanneln, (
        msg->data + (t_data_off += sizeof(prefs->outDevIdx))
    ), sizeof(prefs->inChanneln));
    prefs->inChanneln = ntohl(prefs->inChanneln);

    memcpy(&prefs->outChanneln, (
        msg->data + (t_data_off += sizeof(prefs->inChanneln))
    ), sizeof(prefs->outChanneln));
    prefs->outChanneln = ntohl(prefs->outChanneln);

    memcpy(&prefs->framesPerBuffer, (
        msg->data + (t_data_off += sizeof(prefs->outChanneln))
    ), sizeof(prefs->framesPerBuffer));
    prefs->framesPerBuffer = ntohl(prefs->framesPerBuffer);
  
    int tmpi;
    memcpy(&tmpi, msg->data + (
        t_data_off += sizeof(prefs->framesPerBuffer)
    ), sizeof(tmpi));
    tmpi = ntohl(tmpi);   // cheat to get the correct byte-ordering
    memcpy(&prefs->sampleRate, &tmpi, sizeof(prefs->sampleRate));
     
    fprintf(stdout, BA_INFO " parsed updated prefs "); 
}


void parseAmpValuesFromMsg(ampValues_t* ampValues, message_t* msg)
{
    if (msg->length != 0x10)
        fprintf(stderr, "\e[31mupdate amp values content length should be 16 bytes.\e[0m\n");

    int t_data_off = msg->data_off;

    memcpy(&ampValues->gain, (
        msg->data + t_data_off
    ), sizeof(ampValues->gain));
    ampValues->gain = ntohl(ampValues->gain);

    memcpy(&ampValues->bass, (
        msg->data + (t_data_off += sizeof(ampValues->gain))
    ), sizeof(ampValues->bass));
    ampValues->bass = ntohl(ampValues->bass);

    memcpy(&ampValues->treble, (
        msg->data + (t_data_off += sizeof(ampValues->bass))
    ), sizeof(ampValues->treble));
    ampValues->treble = ntohl(ampValues->treble);

    memcpy(&ampValues->power, (
        msg->data + (t_data_off += sizeof(ampValues->treble))
    ), sizeof(ampValues->power));
    ampValues->power = ntohl(ampValues->power);
 
    fprintf(stdout, BA_INFO " parsed updated amp values "); 
}

