#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include <dirent.h>
#include <assert.h>

#include <string.h>
#include <stdio.h>
#include <signal.h>

#include <iostream>
#include <string>

#include "common.h"

int main(int argc, const char *argv[])
{
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(9090);

    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int rt1 = bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (rt1 < 0) {
        std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]"<< "bind failed." << std::endl;
        exit(-1);
    }

    int rt2 = listen(listenfd, 65535);
    if (rt2 < 0) {
        std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "listen failed." << std::endl;
        exit(-1);
    }

    signal(SIGPIPE, SIG_IGN);

    int connfd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    do {
        while ((connfd = accept(listenfd, (struct sockaddr *) &client_addr, &client_len)) >= 0) {
            char  head_buf[3];
            char  send_buf[256];
            int send_len = 0;
            const char *p_error = "Server Internal Error, try again later...\n";

            do {
                int read_len = readNum(connfd, head_buf, 3);
                if (read_len == 0) {
                    std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]"<< "client quit." << std::endl;
                    break;
                }
                if (read_len < 0) {
                    std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "readNum error:" << read_len << ", errno:" << errno << std::endl;
                    exit(-1);
                }
                uint16_t len = *(uint16_t *)(&head_buf[0]);
                len = ntohs(len);
                int cmd = head_buf[2];

                if (cmd == CMD_CD) {
                    if (len <= 1 || len >= 256) {
                        std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]"<< "cmd ls error:" << cmd << std::endl;
                        shutdown(connfd, SHUT_WR);
                        break;
                    }

                    --len;

                    char param_buf[256];
                    int param_len = readNum(connfd, param_buf, len);
                    if (param_len == 0) {
                        std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "client quit." << std::endl;
                        break;
                    }
                    if (param_len < 0) {
                        std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "readNum error:" << read_len << ", errno:" << errno << std::endl;
                        exit(-1);
                    }
                    param_buf[param_len] = 0;

                    int ch_ret = chdir(param_buf);
                    if (ch_ret < 0) {
                        assert((send_len = genSendSmallString(send_buf, 256, "server chdir error.\n")) > 0);
                        std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "server chdir error, param:" << param_buf << ", len:" << len << ", errno:" << errno << std::endl;
                        if (send(connfd, (char *) &send_buf, send_len, 0) < 0) {
                            std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "send failed." << std::endl;
                            shutdown(connfd, SHUT_WR);
                            break;
                        }
                    } else {
                        assert((send_len = genSendSmallString(send_buf, 256, "")) > 0);
                        if (send(connfd, (char *) &send_buf, send_len, 0) < 0) {
                            std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "send failed." << std::endl;
                            shutdown(connfd, SHUT_WR);
                            break;
                        }
                    }
                } else if (cmd == CMD_LS) {
                    if (1 != len) {
                        std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "cmd ls error:" << cmd << std::endl;
                        shutdown(connfd, SHUT_WR);
                        break;
                    }

                    // exception
                    char dir_path[256];
                    int get_len = getExecutableDir(dir_path, 256);
                    if (get_len < 0) {
                        assert((send_len = genSendSmallString(send_buf, 256, p_error)) > 0);

                        if (send(connfd, (char *) &send_buf, send_len, 0) < 0) {
                            std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "send failed." << std::endl;
                            shutdown(connfd, SHUT_WR);
                            break;
                        }

                        continue;
                    }

                    DIR* dp;
                    struct dirent* ep;
                    dp = opendir("./");
                    if (dp == NULL) {
                        assert((send_len = genSendSmallString(send_buf, 256, "open dir error.\n")) > 0);
                        if (send(connfd, (char *) &send_buf, send_len, 0) < 0) {
                            std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "send failed." << std::endl;
                            shutdown(connfd, SHUT_WR);
                            break;
                        }

                        continue;
                    } else {
                        std::string send_str = "";
                        while (ep = readdir(dp)) {
                            send_str += ep->d_name;
                            send_str.push_back('\n');
                        }
                        uint32_t total_len = send_str.size();;
                        total_len = htonl(total_len);

                        if (send(connfd, (char *) &total_len, 4, 0) < 0) {
                            std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]"<< "send failed." << std::endl;
                            shutdown(connfd, SHUT_WR);
                            break;
                        }
                        if (send(connfd, send_str.c_str(), send_str.size(), 0) < 0) {
                            std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "send failed." << std::endl;
                            shutdown(connfd, SHUT_WR);
                            break;
                        }

                        closedir(dp);
                    }
                } else if (cmd == CMD_PWD) {
                    if (1 != len) {
                        std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "cmd pwd error:" << cmd << std::endl;
                        shutdown(connfd, SHUT_WR);
                        break;
                    }

                    char dir_path[256];
                    char *cwd_res = getcwd(dir_path, 256);
                    const char *pp = NULL;
                    if (cwd_res == NULL) {
                        pp = "Server Internal Error, try again later...\n";
                    } else {
                        pp = dir_path;
                        int tmp_len = strlen(dir_path);
                        dir_path[tmp_len] = '\n';
                        dir_path[tmp_len + 1] = '\0';
                    }
                    assert((send_len = genSendSmallString(send_buf, 256, pp)) > 0);

                    if (send(connfd, (char *) &send_buf, send_len, 0) < 0) {
                        std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "send failed." << std::endl;
                        shutdown(connfd, SHUT_WR);
                        break;
                    }
                } else {
                    std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "cmd error:" << cmd << std::endl;
                    shutdown(connfd, SHUT_WR);
                    break;
                }
            } while(1);
        }

        if (errno != EINTR) {
            std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]" << "stop server because of errno:" << errno << std::endl;
            break;
        }
    } while(1);

    return 0;
}
