#include "pidList.hpp"

pidList::pidList(pthread_t id)
{
    this->next = NULL;
    this->id = id;
}
pthread_t pidList::getId()
{
    return this->id;
}
void pidList::addToList(pthread_t id)
{
    if (this->id != id)
    {
        if (this->next != NULL)
        {
            this->next->addToList(id);
        }
        else
        {
            this->next = new pidList(id);
        }
    }
}

pidList *pidList::getNext()
{
    return this->next;
}