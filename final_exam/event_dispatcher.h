#ifndef NET_STUDY_EVENT_DISPATCHER
#define NET_STUDY_EVENT_DISPATCHER

#include "thread_pool.h"

#include <pthread.h>

class EventDispatcher {
    public:
        EventDispatcher(ThreadPool  *_threadPool);

        int addConsumer(int fd);
        int removeConsumer(int fd);
        int startEventLoop();
        void stopEventLoop();

        static void *run(void *args);

    private:
        int epollFd;
        volatile bool bStop;
        pthread_t tid;
        ThreadPool *threadPool;
};

#endif
