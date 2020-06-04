#ifndef NET_STUDY_THREAD_POOL
#define NET_STUDY_THREAD_POOL

#include <pthread.h>

#include <queue>

typedef void *(*ThreadPoolMsgFuncPtr)(void *);

struct ThreadPoolMsg {
    ThreadPoolMsgFuncPtr funcPtr;
    void  *args;
};

class ThreadPool {
    public:
        ThreadPool(int queueSize, int concurrentNum) : maxQueueSize(queueSize), threadNum(concurrentNum) {
            queueMutex = PTHREAD_MUTEX_INITIALIZER;
            queueCond = PTHREAD_COND_INITIALIZER;
        }
        ~ThreadPool(int queueSize, int concurrentNum) : maxQueueSize(queueSize), threadNum(concurrentNum) {
            pthread_mutex_destroy(&queueMutex);
            pthread_cond_destroy(&queueCond);
        }
        void start();
        void stop();

        int pushTask(const ThreadPoolMsg &task);
        int popTask(ThreadPoolMsg &task);

        static void *run(void *args);

    private:
        int threadNum;
        int maxQueueSize;

        volatile bool bStop;

        std::queue<ThreadPoolMsg> blockQueue;

        pthread_mutex_t  queueMutex;
        pthread_cond_t queueCond;

};

#endif
