#include "utils.hpp"
#include "jobQueue.hpp"
using namespace std;

jobQueue *queue = NULL;
bool globalExit;
jobScheduler *pool;
socketLockList *sockets = NULL;
pidList *commThreads = NULL;
pthread_mutex_t queueLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t subjectLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t socketListLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queueEmptyCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t queueFullCond = PTHREAD_COND_INITIALIZER;
int blockSize = -1;
int queueSize = 0;
int queueLimit;

void sigintHandler(int sig){
    jobQueue* currQueueNode = NULL;
    jobQueue* tempQueueNode = NULL;

    socketLockList* currSocketNode = NULL;
    socketLockList* tempSocketNode = NULL;

    pidList* currPidNode = NULL;
    pidList* tempPidNode = NULL;
    cout<<"\nDELETING\n";

    // Wait for all workers to finish and exit
    globalExit = true;
    pthread_cond_broadcast(&queueEmptyCond);
    pthread_cond_broadcast(&queueFullCond);
    for(int i = 0; i < pool->getThreads(); i++){
        pthread_join((pool->getThreadIds())[i], NULL);
    }

    // Then, wait for all comms threads
    currPidNode = commThreads;
    commThreads = NULL;
    while(currPidNode != NULL){
        pthread_join(currPidNode->getId(), NULL);
        tempPidNode = currPidNode;
        currPidNode = currPidNode->getNext();
        delete tempPidNode;
        tempPidNode = NULL;
    }
    currPidNode = NULL;

    // Delete the global data
    // First, delete the job queue
    currQueueNode = queue;
    queue = NULL;
    while(currQueueNode != NULL){
        tempQueueNode = currQueueNode;
        currQueueNode = currQueueNode->getNext();
        delete tempQueueNode;
        tempQueueNode = NULL;
    }
    currQueueNode = NULL;

    // Then, delete the delete the socketLockList
    currSocketNode = sockets;
    sockets = NULL;
    while(currSocketNode != NULL){
        tempSocketNode = currSocketNode;
        currSocketNode = currSocketNode->getNext();
        delete tempSocketNode;
        tempSocketNode = NULL;
    }
    currSocketNode = NULL;

    // Finally, delete the scheduler
    delete pool;

    exit(0);
}

void perror_exit(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

void addToQueue(int socket, string filename)
{
    pthread_mutex_lock(&queueLock);
    while (queueSize == queueLimit)
    {
        pthread_cond_wait(&queueFullCond, &queueLock);
    }
    if (queue == NULL)
    {
        queue = new jobQueue(socket, filename);
    }
    else
    {
        queue->addJob(socket, filename);
    }
    queueSize++;
    pthread_cond_signal(&queueEmptyCond);
    pthread_mutex_unlock(&queueLock);
}

void *commThread(void *arg)
{
    int newsock = *(int *)arg;
    char buf[4096+1];
    char * writeBuf = NULL;
    int readSize;
    while ((readSize = read(newsock, buf, 4096)) > 0)
    {
        cout << "buf: " << buf << "\n";
        if (dirExists(buf))
        {
            cout << "Directory Found\n";
            pthread_mutex_lock(&socketListLock);
            if (sockets == NULL)
            {
                sockets = new socketLockList(newsock);
            }
            else
            {
                sockets->addToList(newsock);
            }
            pthread_mutex_unlock(&socketListLock);
            writeBuf = new char[256];
            memset(writeBuf,0,256);
            strcpy(writeBuf,to_string(blockSize).c_str() );
            if (write(newsock, writeBuf, 256) < 0)
            {
                perror_exit((char*)(string("write").c_str()));
            }
            delete[] writeBuf;
            getDirStructure(buf, newsock);
        }
        else
        {
            cout << "Directory does not exist\n";
            if (write(newsock, string("Directory does not exist\n").c_str(), 256) < 0)
            {
                perror_exit((char*)(string("write").c_str()));
            }
            if (write(newsock, string("END\n").c_str(), 256) < 0)
            {
                perror_exit((char*)(string("write").c_str()));
            }
        }
        if (readSize < 1000)
        {
            break;
        }
    }
    return NULL;
}

bool dirExists(char *dir)
{
    struct stat buffer;
    return (stat(dir, &buffer) == 0);
}

void getDirStructure(char *dir, int socket)
{
    char inbuf[256], *temp1, *temp2, *strTokBuff, *lineToken, *tempLineToken, *directoryToken;
    pid_t pid;
    string buff = "", directory, fileName;
    int listenerPipe[2], readReturn;

    // Create Pipe for listener
    if (pipe(listenerPipe) == -1)
    {
        perror(" pipe call ");
        exit(1);
    }
    // Create the listener
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(1);
    }
    else if (pid == 0)
    {
        // Listener Code
        // Set the write end of the pipe to get the output of inotifywait
        close(listenerPipe[0]);
        dup2(listenerPipe[1], STDOUT_FILENO);

        // List files and directories
        if(execl("/bin/ls", "/bin/ls", "-R", dir, NULL) < 0){
            perror("execl");
        }
    }
    else
    {
        memset(inbuf, 0, 256);
        cout<<"GOT HERE\n";
        readReturn = read(listenerPipe[0], inbuf, 256);
        cout<<"GOT HERE\n";
        buff += string(inbuf);
        memset(inbuf, 0, 256);

        while (readReturn == 256)
        {
            readReturn = read(listenerPipe[0], inbuf, 256);
            buff += string(inbuf);
            memset(inbuf, 0, 256);
        }
        if (readReturn < 0)
        {
            perror(" Read from pipe \n");
            exit(1);
        }
        cout << "Read LS: " << buff << "\n####\n";
        strTokBuff = new char[buff.length() + 1];
        strcpy(strTokBuff, buff.c_str());
        lineToken = strtok_r(strTokBuff, "\n", &temp1);
        while (lineToken != NULL)
        {
            tempLineToken = new char[strlen(lineToken) + 1];
            strcpy(tempLineToken, lineToken);
            directoryToken = strtok_r(tempLineToken, ":", &temp2);
            if (directoryToken != NULL && (strcmp(tempLineToken, lineToken) != 0))
            {
                // Line represents a directory
                directory = directoryToken;
            }
            else
            {
                if (!dirExists((char *)(directory+ "/" +string(lineToken) + "/").c_str()))
                {
                    cout<<"Sending file "<<(string(directory) + "/" + string(lineToken))<<" to queue\n";
                    // Line represents a file
                    addToQueue(socket, string(directory) + "/" + string(lineToken));
                }
            }
            lineToken = strtok_r(NULL, "\n", &temp1);
            delete[] tempLineToken;
        }
        delete[] strTokBuff;
    }
}

void createFile(char *fileName, string contents)
{
    int outFile, dirSize;
    string directory = fileName;
    string dirAcum = "";
    char *fileNameBuf, *token, *temp;

    dirSize = string(fileName).find_last_of("/");
    directory.resize(dirSize + 1);
    fileNameBuf = new char[strlen(directory.c_str()) + 1];
    token = NULL;
    strcpy(fileNameBuf, directory.c_str());

    // Create the directory
    if ((directory.c_str())[0] == '.')
    {
        directory.erase(directory.begin());
    }
    cout << "Directory: " << directory << "\n";

    if (!dirExists((char *)(string("./tmp/")).c_str()))
    {
        if (mkdir((string("./tmp/")).c_str(), 0777) < 0)
        {
            perror_exit((char*)(string("mkdir")).c_str());
        }
    }
    dirAcum += "./tmp";

    // Create subdirs
    token = strtok_r(fileNameBuf, "/", &temp);
    while (token != NULL)
    {
        dirAcum += "/" + string(token);
        if (!dirExists((char *)(dirAcum).c_str()))
        {
            if (mkdir((dirAcum).c_str(), 0777) < 0)
            {
                perror_exit((char*)(string("mkdir")).c_str());
            }
        }

        token = strtok_r(NULL, "/", &temp);
    }

    // Wait for directory Creation
    while (!dirExists((char *)(string("./tmp/") + directory).c_str()))
        ;

    // If file exists, delete
    if (dirExists((char *)(string("./tmp/") + string(fileName)).c_str()))
    {
        cout << "Directory Exists: " << (string("./tmp/") + string(fileName)) << "\n";
        if (remove((string("./tmp/") + string(fileName)).c_str()) != 0)
        {
            perror((char*)(string("delete")).c_str());
        }
    }

    outFile = open((string("./tmp/") + string(fileName)).c_str(), O_CREAT | O_RDWR, 0644);
    if (outFile == -1)
    {
        perror(" Creating .out file ");
        exit(1);
    }
    if (write(outFile, contents.c_str(), contents.size()) < 0)
    {
        perror(" Problem with writing ");
        exit(5);
    }
    close(outFile);

    delete[] fileNameBuf;
}

void copyFile(char *fileName, int socket)
{
    char buf[blockSize + 1];
    string fileContents = "";
    cout << "Filename: " << fileName << "\n";

    do
    {
        memset(buf, 0, blockSize + 1);
        if (read(socket, buf, blockSize) < 0)
            perror_exit((char*)(string("read")).c_str());
        if (strcmp(buf, "EOF\n") != 0)
        {
            fileContents = fileContents + string(buf);
        }

    } while (strcmp(buf, "EOF\n") != 0);

    createFile(fileName, fileContents);
}

void sendFile(int socket, string fileName)
{
    char fileNameBuf[4096+1];
    char eofBuf[blockSize+1];
    int textFile, readReturn;
    char msgbuf[blockSize+1];

    memset(msgbuf,0, blockSize+1);
    memset(fileNameBuf, 0,4096 +1);
    memset(eofBuf,0 , blockSize+1);

    strcpy(fileNameBuf,fileName.c_str() );
    strcpy(eofBuf,string("EOF\n").c_str() );

    if (write(socket,fileNameBuf, 4096) < 0)
    {
        perror_exit((char*)(string("write").c_str()));
    }
    textFile = open(fileName.c_str(), O_RDONLY);
    readReturn = read(textFile, msgbuf, blockSize);
    while (readReturn > 0)
    {
        
        if (write(socket, msgbuf, blockSize) < 0)
        {
            perror_exit((char*)(string("write").c_str()));
        }
        memset(msgbuf,0,blockSize+1);
        readReturn = read(textFile, msgbuf, blockSize);
    }
    if (write(socket, eofBuf, blockSize) < 0)
    {
        perror_exit((char*)(string("write").c_str()));
    }
    close(textFile);
}