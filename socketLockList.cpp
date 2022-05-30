#include "socketLockList.hpp"

socketLockList::socketLockList(int socket)
{
    this->next = NULL;
    this->socketMtx = PTHREAD_MUTEX_INITIALIZER;
    this->socket = socket;
}
int socketLockList::getSocket()
{
    return this->socket;
}
void socketLockList::addToList(int givenSocket)
{
    if (this->socket != givenSocket)
    {
        if (this->next != NULL)
        {
            this->next->addToList(givenSocket);
        }
        else
        {
            this->next = new socketLockList(givenSocket);
        }
    }
}
void socketLockList::lock(int givenSocket)
{
    if (this->socket == givenSocket)
    {
        pthread_mutex_lock(&(this->socketMtx));
    }
    else
    {
        this->next->lock(givenSocket);
    }
}
void socketLockList::unlock(int givenSocket)
{
    if (this->socket == givenSocket)
    {
        pthread_mutex_unlock(&(this->socketMtx));
    }
    else
    {
        this->next->unlock(givenSocket);
    }
}