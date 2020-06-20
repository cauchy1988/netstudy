#ifndef NET_STUDY_CHANNEL
#define NET_STUDY_CHANNEL

#include<memory>
#include<mutex>
#include<atomic>
#include<unordered_map>

#include "buffer.h"
#include "event_dispatcher.h"

struct ChannelMsg {
    int fd;
    EventDispatcher *dispatcher;

    ChannelMsg() {}
    ChannelMsg(int _fd, EventDispatcher *_dispatcher) : fd(_fd), dispatcher(_dispatcher) {}
};

class Channel {
public:
    Channel(int fd, bool b_listen = false) : bListen(b_listen), nReadLimit(0), nSendLimit(0) {}

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
    std::atomic<int> nReadLimit;
    std::atomic<int> nSendLimit;

    static std::mutex mapMutex;
    static std::unordered_map<int, std::shared_ptr<Channel>> channelMap;
};

#endif
