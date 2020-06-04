#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "channel.h"

std::mutex Channel::mapMutex;
std::unordered_map<int, shared_ptr<Channel>> Channel::channelMap;

void  Channel::onConnect(int fd, EventDispatcher *dispatcher) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int connfd = accept(fd, (struct sockaddr *) &client_addr, &client_len);
    if (connfd < 0) {
        // tc_error
        return;
    }

    std::shared_ptr<Channel>  one_channel_ptr = new Channel(connfd);

    mapMutex.lock();
    auto insert_result = channelMap.insert({connfd, one_channel_ptr});
    if (!insert_result.second) {
        // tc_error
        channelMap[connfd] = one_channel_ptr;
    }
    mapMutex.unlock();

    dispatcher->addConsumer(connfd);
}

void  Channel::onRead(int fd, EventDispatcher *dispatcher) {

}

void  Channel::onWrite(int fd, EventDispatcher *dispatcher) {

}

void  *Channel::onRead(void *args) {
    ChannelMsg *msg = (ChannelMsg *)(args);
    Channel::onRead(msg->fd, msg->dispatcher);
}

void  *Channel::onWrite(void *args) {
    ChannelMsg *msg = (ChannelMsg *)(args);
    Channel::onWrite(msg->fd, msg->dispatcher);
}


void  Channel::onClose(int fd, EventDispatcher *dispatcher) {
    dispatcher->removeConsumer(fd, false);

    mapMutex.lock();
    channelMap.erase(fd);
    mapMutex.unlock();

    int close_ret = close(fd);
    if (close_ret <  0) {
        // tc_error
    }
}
