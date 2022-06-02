#ifndef JOBQUEUE
#define JOBQUEUE

#include "utils.hpp"
class jobQueue
{
private:
    int socket;
    std::string filename;
    jobQueue *next;

public:
    jobQueue(int givenSocket, std::string givenFileName);
    int getSocket();
    std::string getFilename();
    jobQueue *getNext();
    void addJob(int givenSocket, std::string givenFileName);
    bool isLast(int givenSocket);
};

#endif // JOBQUEUE