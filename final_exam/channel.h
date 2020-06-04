#ifndef NET_STUDY_CHANNEL
#define NET_STUDY_CHANNEL

#include<shared_ptr>
#include<mutex>
#include<atomic>

#include "buffer.h"
#include "event_dispatcher.h"

struct ChannelMsg {
    int fd;
    EventDispatcher *dispatcher;
};

class Channel {
public:
    Channel(int fd, bool b_listen = false) : bListen(b_listen), nLimit(0) {}

public:
    static void onConnect(int fd, EventDispatcher *dispatcher);
    static void onRead(int fd, EventDispatcher *dispatcher);
    static void onWrite(int fd, EventDispatcher *dispatcher);
    static void onClose(int fd, EventDispatcher *dispatcher);

    static void *onRead(void  *args);
    static void *onWrite(void *args);

private:
    int fd;
    Buffer readBuf;
    Buffer sendBuf;
    bool bListen;
    std::atomic<int> nLimit;

    static std::mutex mapMutex;
    static std::unordered_map<int, shared_ptr<Channel>> channelMap;
};

#endif
