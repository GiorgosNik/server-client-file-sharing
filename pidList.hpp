#ifndef PIDLIST
#define PIDLIST

#include "utils.hpp"

class pidList
{
private:
    pidList *next;
    pthread_t id;

public:
    pidList(pthread_t id);
    pthread_t getId();
    void addToList(pthread_t id);
    pidList *getNext();
};

#endif // PIDLIST
