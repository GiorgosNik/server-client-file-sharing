#include "jobScheduler.hpp"
using namespace std;

jobScheduler ::jobScheduler(int execution_threads)
{
    globalExit = false;
    this->execution_threads = execution_threads;
    this->tids = new pthread_t[execution_threads];
    this->subjectSocket = new int[execution_threads];
    memset(this->subjectSocket, -1, execution_threads);
    for (int i = 0; i < this->execution_threads; i++)
    {
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
    for (int i = 0; i < this->execution_threads; i++)
    {
        pthread_join(this->tids[i], NULL);
    }
    delete[] this->tids;
    delete[] this->subjectSocket;
}

void *execute_all_jobs(void *arg)
{
    jobQueue *job = NULL;
    while (1)
    {
        pthread_mutex_lock(&queueLock);

        while (queue == NULL) // Wait while JobQueue is empty
        {
            if (globalExit) // If program exited while this thread was waiting, exit
            {
                pthread_mutex_unlock(&queueLock);
                return NULL;
            }
            pthread_cond_wait(&queueEmptyCond, &queueLock); // wait until queue is no more empty to pop
        }
        job = queue; // Execute Job
        queue = queue->getNext();
        queueSize--;
        pthread_cond_signal(&queueFullCond);
        pool->setSubject(pool->convertId(pthread_self()), job->getSocket());
        pthread_mutex_unlock(&queueLock);
        sockets->lock(job->getSocket());

        sendFile(job->getSocket(), job->getFilename());
        pool->setSubject(pool->convertId(pthread_self()), -1);
        if (pool->chechSubject(job->getSocket()) && (queue == NULL || queue->isLast(job->getSocket())))
        {
            sendFile(job->getSocket(), "END\n");
            cout << "Closing connection.\n";
            close(job->getSocket());
        }

        sockets->unlock(job->getSocket());
        delete job;
    }
    return NULL;
}

void sendFile(int socket, string fileName)
{
    int textFile, readReturn;
    char msgbuf[blockSize+1];
    memset(msgbuf,0,blockSize+1);
    if (write(socket, fileName.c_str(), 256) < 0)
    {
        perror_exit("write");
    }
    textFile = open(fileName.c_str(), O_RDONLY);
    readReturn = read(textFile, msgbuf, blockSize);
    while (readReturn > 0)
    {
        
        if (write(socket, msgbuf, blockSize) < 0)
        {
            perror_exit("write");
        }
        memset(msgbuf,0,blockSize+1);
        readReturn = read(textFile, msgbuf, blockSize);
    }
    if (write(socket, string("EOF\n").c_str(), blockSize) < 0)
    {
        perror_exit("write");
    }
    close(textFile);
}