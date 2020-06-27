#ifndef NET_STUDY_NET_UTILS
#define NET_STUDY_NET_UTILS

#include "buffer.h"

class net_utils {
public:
    static int read(int fd, Buffer buf);
};

#endif
