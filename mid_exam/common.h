#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>

#include <string>

enum {
    CMD_PWD = 0,
    CMD_CD = 1,
    CMD_LS = 2,
    CMD_QUIT = 3
};

int parseCommand(char *buf, uint8_t *cmd, char *param) {
    param[0] = 0;

    while(*buf == ' ') {
        ++buf;
    }
    std::string cmd_str;
    while (*buf != ' ' && *buf != '\n' && *buf) {
        cmd_str.push_back(*buf);
        ++buf;
    }

    // std::cout << "[" << __FILE__ << ":" << __LINE__ << "]"<< "cmd_str:" << cmd_str << ", param:" << param << std::endl;

    if (cmd_str ==  "pwd") {
        *cmd = CMD_PWD;
    } else if (cmd_str == "ls") {
        *cmd = CMD_LS;
    } else if (cmd_str == "cd") {
        *cmd = CMD_CD;

        while (*buf == ' ') {
            ++buf;
        }

        // std::cout << "[" << __FILE__ << ":" << __LINE__ << "]" << "buf:" << buf << std::endl;
        char *pp = param;
        while (*buf != ' ' && *buf != '\n' && *buf) {
            *pp = *buf;
            ++pp;
            ++buf;
        }
        *pp = 0;

        if (0 == strlen(param)) {
            return -1;
        }
    } else if (cmd_str == "quit") {
        *cmd = CMD_QUIT;
    } else {
        return -1;
    }


    return 0;
}

int readNum(int socket_fd, char *buf, size_t size) {
    size_t left = size;
    ssize_t read_len;
    while (left > 0) {
        read_len = recv(socket_fd, buf, left, 0);
        if (read_len == 0) {
            return 0;
        }
        if (read_len < 0) {
            if (errno == EINTR) {
                continue;
            }

            return read_len;
        }

        buf += read_len;
        left -= read_len;
    }

    return size;
}

int getExecutableDir(char *dir, size_t len) {
    char *path_end;
    if (readlink("/proc/self/exe", dir, len) <= 0) {
        return -1;
    }
    path_end = strchr(dir, '/');

    if (path_end == NULL) {
        return -1;
    }

    *(++path_end) = 0;

    return (path_end - dir);
}



int genSendSmallString(char *buf, int size, const char *src) {
    uint32_t len = strlen(src);

    if (len + 4 >= size) {
        return -1;
    }

    *(uint32_t *)(&buf[0]) = htonl(len);
    strncpy(buf + 4, src, size - 4);

    return 4 + len;
}

