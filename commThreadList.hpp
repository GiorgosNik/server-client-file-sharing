#ifndef COMMTHREADIDLIST
#define COMMTHREADIDLIST

#include "utils.hpp"

class commThreadList
{
private:
    pthread_t id;
    commThreadList *next;

public:
    commThreadList(pthread_t givenID);
    void addNode(pthread_t givenID);
    void waitAll();
    pthread_t getID();
    commThreadList *getNext();
};

#endif // COMMTHREADIDLIST