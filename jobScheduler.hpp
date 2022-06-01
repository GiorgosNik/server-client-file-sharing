#ifndef JOBSCHEDULER
#define JOBSCHEDULER

#include "utils.hpp"
#include "jobQueue.hpp"

class jobScheduler
{
private:
    int execution_threads;
    pthread_t *tids;
    int * subjectSocket;
public:
    jobScheduler(int execution_threads);
    const int getThreads() const;
    pthread_t *getThreadIds();
    int convertId(pthread_t givenID);
    void setSubject(int threadId, int socket);
    bool chechSubject(int socket);
    ~jobScheduler();
};

void *execute_all_jobs(void *arg);
#endif //JOBSCHEDULER 