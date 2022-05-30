#include "commThreadList.hpp"
// Is this even needed?
commThreadList::commThreadList(pthread_t givenID)
{
    this->id = givenID;
    this->next = NULL;
}

void commThreadList::addNode(pthread_t givenID){
    if(this->next == NULL){
        this->next = new commThreadList(givenID);
    }else{
        this->next->addNode(givenID);
    }
}

void commThreadList::waitAll(){
    
    if(this->next == NULL){
        return;
    }else{
        // Wait pid
        this->next->waitAll();
    }
}

pthread_t commThreadList::getID()
{
    return this->id;
}
commThreadList *commThreadList::getNext()
{
    return this->next;
}