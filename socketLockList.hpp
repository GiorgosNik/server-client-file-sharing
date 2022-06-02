#ifndef SOCKETLOCKLIST
#define SOCKETLOCKLIST

#include "utils.hpp"

class socketLockList
{
private:
    socketLockList *next;
    int socket;
    pthread_mutex_t socketMtx;

public:
    socketLockList(int socket);
    int getSocket();
    void addToList(int givenSocket);
    void lock(int givenSocket);
    void unlock(int givenSocket);
    socketLockList *getNext();
};

#endif // SOCKETLOCKLIST
