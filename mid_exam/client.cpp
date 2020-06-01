#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <stdio.h>

#include <iostream>

#include "common.h"


int main(int argc, const char *argv[])
{
    if (argc != 2) {
        std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "usage: tcpclient <IPaddress>" << std::endl;
        exit(-1);
    }

    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9090);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    socklen_t server_len = sizeof(server_addr);
    int connect_rt = connect(socket_fd, (struct sockaddr *) &server_addr, server_len);
    if (connect_rt < 0) {
        std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "connect failed";
        exit(-1);
    }

    char buf[128];
    char param[128];
    char write_buf[256];
    char read_buf[256];
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        uint8_t cmd = 0;
        if ((parseCommand(buf, &cmd, param)) < 0) {
            std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "invalid command:" << buf << std::endl;
            continue;
        }

        if (CMD_QUIT == cmd) {
            std::cout << "[" << __FILE__ << ":" << __LINE__ << "]"<< "client going to quit..." << std::endl;
            shutdown(socket_fd, SHUT_WR);
            break;
        }

        // prepare send stream;
        uint16_t len = 1 + strlen(param);
        *(uint16_t *)(write_buf) = htons(len);
        *(uint8_t *)(write_buf + 2) = cmd;
        strncpy(write_buf + 3, param, 128);

        if (send(socket_fd, write_buf, 3 + strlen(param), 0) < 0) {
            std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "send failed." << std::endl;
            exit(-1);
        }

        int msg_len = 0;
        int read_len = readNum(socket_fd, (char *)(&msg_len), 4);
        if (read_len <= 0) {
            std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "readNum error:" << read_len << std::endl;
            exit(-1);
        }
        msg_len = ntohl(msg_len);

        if (msg_len > 0) {
            int left = msg_len;
            while (left > 0) {
                int read_len = readNum(socket_fd, read_buf, 255 < left ? 255 : left);
                if (read_len <= 0) {
                    std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "readLine error:" << read_len << std::endl;
                    exit(-1);
                }
                left -= read_len;

                read_buf[read_len] = 0;
                std::cout << read_buf;
            }
        }
    }

    return 0;
}
