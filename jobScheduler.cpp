#include "jobScheduler.hpp"
using namespace std;

jobScheduler ::jobScheduler(int execution_threads)
{
    globalExit = false;
    this->execution_threads = execution_threads;
    this->tids = new pthread_t[execution_threads];
    this->subjectSocket = new int[execution_threads];
    for (int i = 0; i < this->execution_threads; i++)
    {
        this->subjectSocket[i] = -1;
        pthread_create(&this->tids[i], NULL, &execute_all_jobs, NULL);
    }
}

const int jobScheduler ::getThreads() const
{
    return this->execution_threads;
}

pthread_t *jobScheduler ::getThreadIds()
{
    return this->tids;
}

int jobScheduler ::convertId(pthread_t givenID)
{
    for (int i = 0; i < this->execution_threads; i++)
    {
        if (this->tids[i] == givenID)
        {
            return i;
        }
    }
    return -1;
}

void jobScheduler::setSubject(int threadId, int socket)
{
    this->subjectSocket[threadId] = socket;
}

bool jobScheduler::chechSubject(int socket)
{
    for (int i = 0; i < this->execution_threads; i++)
    {
        if (this->subjectSocket[i] == socket)
        {
            return false;
        }
    }
    return true;
}

jobScheduler ::~jobScheduler()
{
    delete[] this->tids;
    delete[] this->subjectSocket;
}

void *execute_all_jobs(void *arg)
{
    jobQueue *job = NULL;
    pthread_t self = pthread_self();

    while (1)
    {
        pthread_mutex_lock(&queueLock);

        while (queue == NULL) // Wait while the queue is empty
        {
            if (globalExit) // If program exited while this thread was waiting, exit
            {
                pthread_mutex_unlock(&queueLock);
                return NULL;
            }
            pthread_cond_wait(&queueEmptyCond, &queueLock); // wait until queue is no longer empty
        }
        job = queue; // Execute Job
        queue = queue->getNext();
        queueSize--;

        // Signal any comm threads waiting to add to the queue
        pthread_cond_signal(&queueFullCond);

        // Change the socket this thread is working on
        pthread_mutex_lock(&subjectLock);
        pool->setSubject(pool->convertId(pthread_self()), job->getSocket());
        pthread_mutex_unlock(&subjectLock);
        pthread_mutex_unlock(&queueLock);

        sockets->lock(job->getSocket());
        cout << "[Thread " << self << "]: Worker received file to send: " << job->getFilename() << "\n";

        // Send the file
        sendFile(job->getSocket(), job->getFilename());
        pthread_mutex_lock(&subjectLock);
        pool->setSubject(pool->convertId(pthread_self()), -1);

        // If this is the last file close the connection
        if (pool->chechSubject(job->getSocket()) && (queue == NULL || queue->isLast(job->getSocket())))
        {
            sendFile(job->getSocket(), "END\n");
            cout << "Closing connection.\n";
            close(job->getSocket());
        }
        pthread_mutex_unlock(&subjectLock);
        sockets->unlock(job->getSocket());
        delete job;
    }
    return NULL;
}
