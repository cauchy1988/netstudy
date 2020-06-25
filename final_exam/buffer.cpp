#include <string.h>

#include "buffer.h"

int Buffer::readFromBuffer(char *_buf, int len, bool bChange) {
    if (len < 0) {
        return -1;
    }

    int real_size = len < size() ? len : size();
    if (rearIdx < frontIdx) {
        if (real_size >= preSize()) {
            int pre_size = preSize();
            memcpy(_buf, front(), pre_size);

            int rem = real_size - pre_size;
            memcpy(_buf + pre_size, buf, rem);
        } else {
            memcpy(_buf, front(), real_size);
        }
    } else {
        memcpy(_buf, front(), real_size);
    }

    if (bChange) {
        frontIdx = (frontIdx + real_size) % (capacity + 1);
    }

    return real_size;
}


int Buffer::readFromBufferLine(char *_buf, int len, bool bChange) {

}

int Buffer::writeToBuffer(char *_buf, int len) {
    if (!enLarge()) {
        return -1;
    }

    int back_rem = capacity + 1 - rearIdx;
    if (back_rem >= len) {
        memcpy(rear(), _buf, len);
    } else {
        memcpy(rear(), _buf, back_rem);
        memcpy(buf, _buf + back_rem, len - back_rem);
    }

    rearIdx = (rearIdx + len) % (capacity + 1);

    return len;
}
