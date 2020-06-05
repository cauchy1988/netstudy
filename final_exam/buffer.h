#ifndef NET_STUDY_BUFFER
#define NET_STUDY_BUFFER

class Buffer {
    public:
       Buffer(int _capacity) : capacity(_capacity) {
           buf = new char[_capacity + 1];
           assert(buf);

           front = rear = 0;
       }

       bool full() {
           return (rear + 1) % (capacity + 1) == front;
       }

       bool empty() {
           return rear == front;
       }

       int size() {
           if (front <= rear) {
               return rear - front;
           } else {
               return (rear + 1) + (capacity + 1 - front);
           }
       }

       int remain() {
           return capacity - size();
       }

       char *rear() {
           return &buf[rear];
       }

       char *front() {
           return &buf[front];
       }

       int readFromBuffer(char *_buf, int len, bool bChange = false);

       int writeToBuffer(char *_buf, int len);

    private:
       char *buf;
       int capacity;

       int front;
       int rear;
};

#endif
