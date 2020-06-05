#include "thread_pool.h"

void ThreadPool::start() {
    bStop = false;

    for (int i = 0; i < threadNum; ++i) {
        pthread_t tid;
        pthread_create(&tid, NULL, run, (void  *)this);
        pthread_detach(tid);
    }
}

void ThreadPool::stop() {
    bStop = true;
}

void *ThreadPool::run(void *args) {
    ThreadPool *pool = (ThreadPool *)(args);

    ThreadPoolMsg one_msg;
    while (!pool->bStop) {
        int pop_ret = pool->popTask(one_msg);
        if (pop_ret) {
            continue;
        }

        one_msg.funcPtr(one_msg.args);
    }
}


int ThreadPool::pushTask(const ThreadPoolMsg &task) {
    bool isEmpty = false;

    pthread_mutex_lock(&queueMutex);
    isEmpty = blockQueue.empty();

    while (blockQueue.size() >= maxQueueSize) {
        if(pthread_cond_wait(&fullCond, &queueMutex)) {
            pthread_mutex_unlock(&queueMutex);
            //tc_error
            return -1;
        }
    }

    blockQueue.push(task);
    pthread_mutex_unlock(&queueMutex);
    if (isEmpty) {
        pthread_cond_broadcast(&this->emptyCond);
    }

    return 0;
}

int ThreadPool::popTask(ThreadPoolMsg &task) {
    bool isFull = false;

    pthread_mutex_lock(&queueMutex);
    isFull = blockQueue.size() >= maxQueueSize;

    while(blockQueue.empty()) {
        if(pthread_cond_wait(&emptyCond, &queueMutex)) {
            pthread_mutex_unlock(&queueMutex);
            //tc_error
            return -1;
        }
    }

    task = blockQueue.front();
    blockQueue.pop();
    pthread_mutex_unlock(&queueMutex);
    if (isFull) {
        pthread_cond_broadcast(&this->fullCond);
    }

    return 0;
}
