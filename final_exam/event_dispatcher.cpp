#include <sys/epoll.h>
#include <assert.h>
#include <cstddef>
#include <stdlib.h>
#include <errno.h>

#include "event_dispatcher.h"
#include "channel.h"

const int MAX_EVENT_NUM = 1024;

EventDispatcher::EventDispatcher(ThreadPool  *_threadPool) {
    assert(_threadPool);
    this->threadPool = _threadPool;

    epollFd = epoll_create(1);
    if (epollFd < 0) {
        //tc_error
        assert(false);
    }
}

int EventDispatcher::addConsumer(int fd) {
    struct epoll_event ev;
    ev.events |= (EPOLLIN | EPOLLRDHUP | EPOLLET);
    ev.data.fd = fd;

    int ctl_ret = epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev);
    if (ctl_ret < 0) {
        // tc_error
        return -1;
    }

    return 0;
}

int EventDispatcher::removeConsumer(int fd) {
    int ctl_ret = epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
    if (ctl_ret < 0) {
        // tc_error
        return -1;
    }

    return 0;
}

int EventDispatcher::startEventLoop() {
    bStop = false;

    int thread_create_ret = pthread_create(&tid, NULL, run, (void *)this);
    if (thread_create_ret < 0) {
        //tc_error
    }

    return 0;
}

void EventDispatcher::stopEventLoop() {
    bStop = true;

    int join_ret = pthread_join(tid, NULL);
    if (join_ret < 0) {
        //tc_error
    }
}

void  *EventDispatcher::run(void *args) {
    EventDispatcher *pDispatcher = (EventDispatcher *)(args);
    struct epoll_event evList[MAX_EVENT_NUM];
    int ready = 0;

    while (!pDispatcher->bStop) {
        ready = epoll_wait(epollFd, evList, MAX_EVENT_NUM, -1);

        if (ready == -1) {
            if (errno == EINTR) {
               continue;
            } else {
                // tc_error
                exit(-1);
            }
        }

        for (int j = 0; j < ready; ++j) {
            struct epoll_event &one_ev = evList[j];
            if (one_ev.events & EPOLLIN) {
                //Channel::onRead(one_ev.data.fd, pDispatcher);
                //add to thread pool
                struct ChannelMsg  *channelMsg = new ChannelMsg(one_ev.data.fd, pDispatcher);
                struct ThreadPoolMsg poolMsg;
                poolMsg.funcPtr = Channel::onRead;
                poolMsg.args = (void *)(channelMsg);

                threadPool->pushTask(poolMsg);
            } else if (one_ev.events & (EPOLLHUP | EPOLLERR)) {
                Channel::onClose(one_ev.data.fd, pDispatcher);
                int close_ret = close(one_ev.data.fd);
                if (close_ret < 0) {
                    if (errno == EINTR) {
                        close(one_ev.data.fd);
                    }

                    //tc_error
                }
            } else if (one_ev.events & EPOLLOUT) {
                //Channel::onWrite(one_ev.data.fd, pDispatcher);
                //add to thread pool
                struct ChannelMsg  *channelMsg = new ChannelMsg(one_ev.data.fd, pDispatcher);
                struct ThreadPoolMsg poolMsg;
                poolMsg.funcPtr = Channel::onWrite;
                poolMsg.args = (void *)(channelMsg);

                threadPool->pushTask(poolMsg);
            } else {
                //tc_error
            }
        }
    }
}
