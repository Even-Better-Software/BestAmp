#ifndef BA_PORTAUDIO_HELPERS
#define BA_PORTAUDIO_HELPERS

#include <portaudio.h>


/**
dump struct fields to stdout.
*/
void printHostApiInfo(PaHostApiInfo* h);

/**
dump struct fields to stdout.
*/
void printDeviceInfo(PaDeviceInfo* d);


#endif
