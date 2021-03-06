//
// Created by tangchao on 2020/6/25.
//

#include "net_utils.h"

int net_utils::read(int fd, Buffer buf) {
    char local_buf[1024];
    int len = 0;

    while (true) {
        int read_ret = read(fd, local_buf, 1024);
        if (read_ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return read_ret;
            } else if (errno == EINTR) {
                continue;
            } else {
                // tc_error
                return read_ret;
            }
        }

        if (read_ret == 0) {
            return 0;
        }

        int write_ret = buf.writeToBuffer(local_buf, read_ret);
        if (write_ret < 0) {
            // tc_error
            return write_ret;
        }

        len += write_ret;
    }
}
