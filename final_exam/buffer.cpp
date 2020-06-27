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


/*
 * read characters from this Buffer until meet a \n or \r or \r\n in the buffer
 * an extra \0 will be inputed in the end of buf
 * return value: the length of the readed characters exclude \0
 */
int Buffer::readFromBufferLine(char *_buf, int len, bool bChange) {
    if (len <= 0) {
       return -1;
    }

    int i = 0;
    int j = frontIdx;
    int buf_end = frontIdx <= rearIdx ? rearIdx : capacity + 1;
    char c = '\0';

    while (i < len - 1 && c != '\n' && j < buf_end) {
        c = _buf[j];
        if (c == '\r') {
           if (j + 1 < buf_end && _buf[j + 1] == '\n' || (j + 1 >= buf_end && frontIdx > rearIdx && rearIdx > 0 && _buf[0] == '\n')) {
               j = (j + 1 < buf_end ? j + 1 : 0);
           }
           c = '\n';
        }

        buf[i++] = c;
        ++j;
    }

    if (frontIdx > rearIdx && i < len - 1 && c != '\n') {
       j = 0;
       while (i < len - 1 && c != '\n' && j < rearIdx) {
           c = _buf[j];
           if (c == '\r') {
               if (j + 1 < rearIdx && _buf[j + 1] == '\n') {
                   ++j;
               }
               c = '\n';
           }

           buf[i++] = c;
           ++j;
       }
    }

    buf[i] = '\0';

    if (bChange) {
        frontIdx = j < capacity + 1 ? j : 0;
    }

    return i;
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
