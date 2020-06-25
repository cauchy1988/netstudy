#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "channel.h"
#include "net_utils.h"

std::mutex Channel::mapMutex;
std::unordered_map<int, std::shared_ptr<Channel>> Channel::channelMap;

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
    mapMutex.lock();
    auto find_iter = channelMap.find(fd);
    if (find_iter == channelMap.end()) {
        mapMutex.unlock();
        onClose(fd, dispatcher);

        //tc_error
        return;
    }
    auto channel_ptr = find_iter->second;
    mapMutex.unlock();

    // 保证被唤醒的描述字一次只能被一个线程读取，并且保证在edge trigger下，套接字的缓存被顺利读取、及时读尽
    if (channel_ptr->nReadLimit.fetch_add(1, std::memory_order_relaxed) == 0) {
        int current_num = channel_ptr->nReadLimit.load(std::memory_order_relaxed);

        do {
            if (channel_ptr->bListen) {
                onConnect(fd,  dispatcher);
            } else {
                int read_ret = net_utils::read(channel_ptr->fd, channel_ptr->readBuf);
                if (read_ret == 0 || read_ret < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    // cope remainders
                    return onClose(fd, dispatcher);
                }

                // cope with binary to construct possible request
            }
        } while (channel_ptr->nReadLimit.compare_exchange_strong(current_num, 0, std::memory_order_relaxed));

    }
}

void  Channel::onWrite(int fd, EventDispatcher *dispatcher) {
    mapMutex.lock();
    auto find_iter = channelMap.find(fd);
    if (find_iter == channelMap.end()) {
        mapMutex.unlock();
        onClose(fd, dispatcher);

        //tc_error
        return;
    }
    auto channel_ptr = find_iter->second;
    mapMutex.unlock();

    if (channel_ptr->nWriteLimit.fetch_add(1, std::memory_order_relaxed) == 0) {


        channel_ptr->nWriteLimit.store(0, std::memory_order_relaxed);
    }
}

void  *Channel::onRead(void *args) {
    ChannelMsg *msg = (ChannelMsg *)(args);
    Channel::onRead(msg->fd, msg->dispatcher);
    delete msg;
}

void  *Channel::onWrite(void *args) {
    ChannelMsg *msg = (ChannelMsg *)(args);
    Channel::onWrite(msg->fd, msg->dispatcher);
    delete msg;
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
