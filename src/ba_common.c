#include "ba_common.h"


int bufRealloc(char** buf, size_t bufSize)
{
    size_t nbufSize = 2 * bufSize;
    if (nbufSize >= BA_BUFLEN_ABS) {
        LERROR("buffer reallocation would exceeds maximum possible buffer size");
        return -1;
    }
    LINFO("buffer reallocation");
    *buf = realloc(*buf, nbufSize);
    return nbufSize;
}

