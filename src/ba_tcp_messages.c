#include "ba_tcp_messages.h"


void printMessage(message_t* msg)
{
    printf("type: %d, length: %d\n", msg->type, msg->length);
}


int parseMessage(message_t* pMsg, char* msg)
{
    int32_t typeByte = 0;
    memcpy(&typeByte, msg, BA_NUMERIC_FIELD_B);
    typeByte = ntohl(typeByte);

    int32_t lengthByte = 0;
    memcpy(&lengthByte, (msg + BA_NUMERIC_FIELD_B), BA_NUMERIC_FIELD_B);
    lengthByte = ntohl(lengthByte);

    switch (typeByte) {
    case KILL:
        pMsg->type = KILL;
        break;
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
    } else if (lengthByte > 0) {
        pMsg->length = lengthByte;
        pMsg->data_off = BA_HEADER_SIZE_B;
    }

    return 0;
}


void parsePrefsFromMsg(baPrefs_t* prefs, message_t* msg, char* rbuf)
{
    if (msg->length != 0x24)
        fprintf(stderr, "\e[31mupdate prefs content length should be 36 bytes.\e[0m\n");

    int t_data_off = msg->data_off;

    memcpy(&prefs->hostApiIdx, (rbuf + t_data_off), BA_NUMERIC_FIELD_B);
    prefs->hostApiIdx = ntohl(prefs->hostApiIdx);
    t_data_off += BA_NUMERIC_FIELD_B;

    memcpy(&prefs->inDevIdx, (rbuf + t_data_off), BA_NUMERIC_FIELD_B);
    prefs->inDevIdx = ntohl(prefs->inDevIdx);
    t_data_off += BA_NUMERIC_FIELD_B;

    memcpy(&prefs->outDevIdx, (rbuf + t_data_off), BA_NUMERIC_FIELD_B);
    prefs->outDevIdx = ntohl(prefs->outDevIdx);
    t_data_off += BA_NUMERIC_FIELD_B;

    memcpy(&prefs->inChanneln, (rbuf + t_data_off), BA_NUMERIC_FIELD_B);
    prefs->inChanneln = ntohl(prefs->inChanneln);
    t_data_off += BA_NUMERIC_FIELD_B;

    memcpy(&prefs->outChanneln, (rbuf + t_data_off), BA_NUMERIC_FIELD_B);
    prefs->outChanneln = ntohl(prefs->outChanneln);
    t_data_off += BA_NUMERIC_FIELD_B;

    memcpy(&prefs->framesPerBuffer, (rbuf + t_data_off), BA_NUMERIC_FIELD_B);
    prefs->framesPerBuffer = ntohl(prefs->framesPerBuffer);
    t_data_off += BA_NUMERIC_FIELD_B;
 
    int64_t tmpi;
    memcpy(&tmpi, (rbuf + t_data_off), BA_DOUBLE_FIELD_B);
    tmpi = ntohll(tmpi);
    memcpy(&prefs->sampleRate, &tmpi, sizeof(prefs->sampleRate));
    t_data_off += BA_DOUBLE_FIELD_B;
     
    fprintf(stdout, BA_INFO " parsed updated prefs\n"); 
}


void parseAmpValuesFromMsg(ampValues_t* ampValues, message_t* msg, char* rbuf)
{
    if (msg->length != 0x14)
        fprintf(stderr, "\e[31mupdate amp values content length should be 20 bytes.\e[0m\n");

    int t_data_off = msg->data_off;
    int32_t t_gain;
    int32_t t_bass;
    int32_t t_treble;
    int32_t t_power;
    int32_t t_raw_voice;

    memcpy(&t_gain, (rbuf + t_data_off), BA_NUMERIC_FIELD_B);
    t_gain = ntohl(t_gain);
    t_data_off += BA_NUMERIC_FIELD_B;

    memcpy(&t_bass, rbuf + t_data_off, BA_NUMERIC_FIELD_B);
    t_bass = ntohl(t_bass);
    t_data_off += BA_NUMERIC_FIELD_B;

    memcpy(&t_treble, rbuf + t_data_off, BA_NUMERIC_FIELD_B);
    t_treble = ntohl(t_treble);
    t_data_off += BA_NUMERIC_FIELD_B;

    memcpy(&t_power, rbuf + t_data_off, BA_NUMERIC_FIELD_B);
    t_power = ntohl(t_power);
    t_data_off += BA_NUMERIC_FIELD_B; 

    memcpy(&t_raw_voice, rbuf + t_data_off, BA_NUMERIC_FIELD_B);
    t_raw_voice = ntohl(t_raw_voice);
    t_data_off += BA_NUMERIC_FIELD_B;

    // [WO] set the voice first as voice struct properties are used to
    // determine the actual db ranges of the dial values
    setAmpVoiceType(ampValues, t_raw_voice);
    setAmpGain(ampValues, t_gain);
    setAmpBass(ampValues, t_bass);
    setAmpTreble(ampValues, t_treble);
    setAmpPower(ampValues, t_power);
 
    fprintf(stdout, BA_INFO " parsed updated amp values\n"); 
}


void parseDeviceQueryFromMsg(int* hostApi, message_t* msg, char* rbuf)
{
    if (msg->length != 0x04)
        fprintf(stderr, "\e[31mquery host apis content length should be 4 bytes\e[0m\n");

    int t_data_off = msg->data_off;
    int32_t t = 0;
    memcpy(&t, (
        rbuf + t_data_off
    ), BA_NUMERIC_FIELD_B);
    *hostApi = ntohl(t);

    fprintf(stderr, BA_INFO " parsed device query\n");
}


void badMessageReply(message_t* rpmsg, char* wbuf)
{
    if (wbuf == NULL) {
        fprintf(stderr, BA_WRITE_BUFFER_NULL);
        return;
    }
    int offset = 0;
    int32_t type    = rpmsg->type   = BAD;
    int32_t length  = rpmsg->length = 0;
    int32_t htype   = htonl(type);
    int32_t hlength = htonl(length);
    memcpy(wbuf + offset, (void*)&htype, BA_NUMERIC_FIELD_B);
    offset += BA_NUMERIC_FIELD_B;
    memcpy(wbuf + offset, (void*)&hlength, BA_NUMERIC_FIELD_B);
    offset += BA_NUMERIC_FIELD_B;
}


void byeMessageReply(message_t* rpmsg, char* wbuf)
{
    if (wbuf == NULL) {
        fprintf(stderr, BA_WRITE_BUFFER_NULL);
        return;
    }
    int32_t type    = rpmsg->type   = BYE;
    int32_t length  = rpmsg->length = 0;
    int32_t htype   = htonl(type);
    int32_t hlength = htonl(length);
    memcpy(wbuf, &htype, BA_NUMERIC_FIELD_B);
    memcpy(wbuf + BA_NUMERIC_FIELD_B, &hlength, BA_NUMERIC_FIELD_B);
}


void genericOkReply(message_t* rpmsg, char* wbuf)
{
    if (wbuf == NULL) {
        fprintf(stderr, BA_WRITE_BUFFER_NULL);
        return;
    }
    int offset = 0;
    int32_t type    = rpmsg->type   = OK;
    int32_t length  = rpmsg->length = 0;
    int32_t htype   = htonl(type);
    int32_t hlength = htonl(length);
    memcpy(wbuf + offset, &htype, BA_NUMERIC_FIELD_B);
    offset += BA_NUMERIC_FIELD_B;
    memcpy(wbuf + offset, &hlength, BA_NUMERIC_FIELD_B);
    offset += BA_NUMERIC_FIELD_B;
}


void updatePreferencesReply(
    message_t* rpmsg, char* wbuf, updatePrefsRep_t* content
) {
    if (wbuf == NULL) {
        fprintf(stderr, BA_WRITE_BUFFER_NULL);
        return;
    }
    int offset = 0;
    int32_t type    = rpmsg->type   = UPDATE_PREFS_REPLY;
    int32_t length  = rpmsg->length = BA_NUMERIC_FIELD_B;
    int32_t htype   = htonl(type);
    int32_t hlength = htonl(length);
    int32_t hcode   = htonl(content->code);
    memcpy(wbuf + offset, &htype, BA_NUMERIC_FIELD_B);
    offset += BA_NUMERIC_FIELD_B;
    memcpy(wbuf + offset, &hlength, BA_NUMERIC_FIELD_B);
    offset += BA_NUMERIC_FIELD_B;
    memcpy(wbuf + offset, &hcode, BA_NUMERIC_FIELD_B);
    offset += BA_NUMERIC_FIELD_B;
}


void updateAmpValuesReply(
    message_t* rpmsg, char* wbuf
) {
    (void)rpmsg;
    (void)wbuf;
    // [WO] this one can use the generic OK reply
    /*
    if (wbuf == NULL) {
        fprintf(stderr, BA_WRITE_BUFFER_NULL);
        return;
    }
    int type = rpmsg->type = UPDATE_AMP_VALS_REPLY;
    int length = rpmsg->length = 
    */
}


void queryHostApisReply(
    message_t* rpmsg, char* wbuf, queryHostApiRep_t* content
) {
    if (wbuf == NULL) {
        fprintf(stderr, BA_WRITE_BUFFER_NULL);
        return;
    }
    int offset = 0;
    int32_t type = rpmsg->type = QUERY_HOST_APIS_REPLY;
    // 4B id, 32B name, 4B default in idx, 4B default out idx
    int32_t length = rpmsg->length = content->hostApiCount * 44;
    int32_t htype = htonl(type);
    int32_t hlength = htonl(length);

    memcpy(wbuf, &htype, BA_NUMERIC_FIELD_B);
    offset += BA_NUMERIC_FIELD_B;
    memcpy(wbuf + offset, &hlength, BA_NUMERIC_FIELD_B);
    offset += BA_NUMERIC_FIELD_B;
   
    // [WO] this is kinda dumb...
    //      but it is true that I is the index of the host API
    for (int i = 0; i < content->hostApiCount; i++) {
        const PaHostApiInfo* info = *(content->hostApis + i);

        int32_t hindex = htonl(i);
        memcpy(wbuf + offset, &hindex, BA_NUMERIC_FIELD_B);
        offset += BA_NUMERIC_FIELD_B;

        // so the name is UP TO 32 bytes long
        // if we just copy 32 bytes from the struct name field, if longer than 32
        // bytes we get whatever comes after that too (of course)
        // fprintf(stderr, "%d\n", (int)strlen(info->name));
        memcpy(wbuf + offset, info->name,
            min(strlen(info->name) + 1, BA_SMALL_STR_FIELD_B)); // copy the 32 bytes of the name
        offset += BA_SMALL_STR_FIELD_B;

        int32_t hdefaultIDevIdx = htonl(info->defaultInputDevice);
        memcpy(wbuf + offset, &hdefaultIDevIdx, BA_NUMERIC_FIELD_B);
        offset += BA_NUMERIC_FIELD_B;

        int32_t hdefaultODevIdx = htonl(info->defaultOutputDevice);
        memcpy(wbuf + offset, &hdefaultODevIdx, BA_NUMERIC_FIELD_B);
        offset += BA_NUMERIC_FIELD_B;
    }
}


void queryDevicesReply(
    message_t* rpmsg, char* wbuf, queryDeviceRep_t* content
) {
    if (wbuf == NULL) {
        fprintf(stderr, BA_WRITE_BUFFER_NULL);
        return;
    }
    int offset = 0;
    int32_t type = rpmsg->type = QUERY_DEVICES_REPLY;
    // 4B idx, 32B name, 4B max in channel, 4B max out channel, 8B default sample rate
    int32_t length = rpmsg->length = content->deviceCount * 52;
    int32_t htype = htonl(type);
    int32_t hlength = htonl(length);

    memcpy(wbuf + offset, &htype, BA_NUMERIC_FIELD_B);
    offset += BA_NUMERIC_FIELD_B;
    memcpy(wbuf + offset, &hlength, BA_NUMERIC_FIELD_B);
    offset += BA_NUMERIC_FIELD_B;

    for (int i = 0; i < content->deviceCount; i++) {
        const PaDeviceInfo* info = *(content->devices + i);
        int32_t deviceGlobalIdx = *(content->deviceIdxes + i);

        int32_t hdeviceGlobalIdx = htonl(deviceGlobalIdx);
        memcpy(wbuf + offset, &hdeviceGlobalIdx, BA_NUMERIC_FIELD_B);
        offset += BA_NUMERIC_FIELD_B;

        memcpy(wbuf + offset, info->name,
            min(strlen(info->name) + 1, BA_SMALL_STR_FIELD_B));
        offset += BA_SMALL_STR_FIELD_B;

        int32_t hdeviceMaxICh = htonl(info->maxInputChannels);
        memcpy(wbuf + offset, &hdeviceMaxICh, BA_NUMERIC_FIELD_B);
        offset += BA_NUMERIC_FIELD_B;

        int32_t hdeviceMaxOCh = htonl(info->maxOutputChannels);
        memcpy(wbuf + offset, &hdeviceMaxOCh, BA_NUMERIC_FIELD_B);
        offset += BA_NUMERIC_FIELD_B;

        int64_t tmp = 0;
        memcpy(&tmp, &(info->defaultSampleRate), BA_DOUBLE_FIELD_B);
        tmp = htonll(tmp);
        memcpy(wbuf + offset, &tmp, BA_DOUBLE_FIELD_B);
        offset += BA_DOUBLE_FIELD_B;
    }
}

void queryDevicesBadReply(
    message_t* rpmsg, char* wbuf
) {
    if (wbuf == NULL) {
        fprintf(stderr, BA_WRITE_BUFFER_NULL);
        return;
    }
    int offset = 0;
    int32_t type = rpmsg->type = QUERY_DEVICES_REPLY;
    int32_t length = rpmsg->length = 4;
    int32_t code = 1;
    int32_t htype = htonl(type);
    int32_t hlength = htonl(length);
    int32_t hcode = htonl(code);

    memcpy(wbuf + offset, &htype, BA_NUMERIC_FIELD_B);
    offset += BA_NUMERIC_FIELD_B;
    memcpy(wbuf + offset, &hlength, BA_NUMERIC_FIELD_B);
    offset += BA_NUMERIC_FIELD_B;
    memcpy(wbuf + offset, &hcode, BA_NUMERIC_FIELD_B);
}

