#include<shared_ptr>
#include<mutex>
#include<atomic>

#include "buffer.h"
#include "event_dispatcher.h"

class Channel {
public:
    Channel(int fd, bool b_listen = false) : bListen(b_listen), nLimit(0) {}

public:
    static void onConnect(int fd, EventDispatcher *dispatcher);
    static void onRead(int fd, EventDispatcher *dispatcher);
    static void onWrite(int fd, EventDispatcher *dispatcher);
    static void onClose(int fd, EventDispatcher *dispatcher);

private:
    int fd;
    Buffer readBuf;
    Buffer sendBuf;
    bool bListen;
    std::atomic<int> nLimit;

    static std::mutex mapMutex;
    static std::unordered_map<int, shared_ptr<Channel>> channelMap;
};
