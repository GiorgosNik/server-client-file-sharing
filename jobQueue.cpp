#include "jobQueue.hpp"
using namespace std;
jobQueue::jobQueue(int givenSocket, std::string givenFileName)
{
    this->socket=givenSocket;
    this->filename = givenFileName;
    this->next = NULL;
}

int jobQueue::getSocket(){
    return this->socket;
}

jobQueue* jobQueue::getNext(){
    return this->next;
}

string jobQueue::getFilename(){
    return this->filename;
}

void jobQueue::addJob(int givenSocket, std::string givenFileName){
    if(this->next != NULL){
        this->next->addJob(givenSocket, givenFileName);
    }else{
        this->next = new jobQueue(givenSocket, givenFileName);
    }
}

bool jobQueue::isLast(int givenSocket){
    if(this->socket != givenSocket){
        if(this->next != NULL){
            return this->next->isLast(givenSocket);
        }else{
            return true;
        }
    }else{
        return false;
    }
}