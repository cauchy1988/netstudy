#ifndef NET_STUDY_BUFFER
#define NET_STUDY_BUFFER

#include <stdlib.h>

class Buffer {
    public:
       Buffer(int _capacity) : capacity(_capacity) {
           buf = new char[_capacity + 1];
           assert(buf);

           frontIdx = rearIdx = 0;
       }

       Buffer() {
           Buffer(2048);
       }

       bool full() {
           return (rearIdx + 1) % (capacity + 1) == frontIdx;
       }

       bool empty() {
           return rearIdx == frontIdx;
       }

       int size() {
           if (frontIdx <= rearIdx) {
               return rearIdx - frontIdx;
           } else {
               return (rearIdx) + (capacity + 1 - frontIdx);
           }
       }

       int remain() {
           return capacity - size();
       }

       char *rear() {
           return &buf[rearIdx];
       }

       char *front() {
           return &buf[frontIdx];
       }

       int preSize() {
           if (rearIdx < frontIdx) {
               return capacity + 1 - frontIdx;
           }

           return -1;
       }

       int suffixSize() {
           if (rearIdx < frontIdx) {
               return rearIdx;
           }

           return -1;
       }

       int readFromBuffer(char *_buf, int len, bool bChange = false);

       int writeToBuffer(char *_buf, int len);

    private:
       char *buf;
       int capacity;

       int frontIdx;
       int rearIdx;
};

#endif
