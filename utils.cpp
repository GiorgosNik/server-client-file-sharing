#include "utils.hpp"
#include "jobQueue.hpp"
using namespace std;
/* Wait for all dead child processes */
jobQueue *queue = NULL;
bool globalExit;
jobScheduler *pool;
socketLockList *sockets = NULL;
pthread_mutex_t queueLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t socketListLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queueEmptyCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t queueFullCond = PTHREAD_COND_INITIALIZER;
int blockSize = -1;
int queueSize = 0;
int queueLimit;

void sigchld_handler(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
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
    char buf[1001];
    struct stat buffer;
    int readSize;
    while ((readSize = read(newsock, buf, 1000)) > 0)
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
            if (write(newsock, to_string(blockSize).c_str(), 256) < 0)
            {
                perror_exit("write");
            }
            getDirStructure(buf, newsock);
        }
        else
        {
            cout << "Directory does not exist\n";
            if (write(newsock, string("Directory does not exist\n").c_str(), 256) < 0)
            {
                perror_exit("write");
            }
            if (write(newsock, string("END\n").c_str(), 256) < 0)
            {
                perror_exit("write");
            }
        }
        if (readSize < 1000)
        {
            return NULL;
        }
    }
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
    int listenerPipe[2], i = 0, size = 0, readReturn;

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
        sleep(1);
        // Listener Code
        // Set the write end of the pipe to get the output of inotifywait
        close(listenerPipe[0]);
        dup2(listenerPipe[1], STDOUT_FILENO);
        close(listenerPipe[1]);

        // List files and directories
        execl("/bin/ls", "ls", "-R", dir);
    }
    else
    {

        memset(inbuf, 0, 256);
        readReturn = read(listenerPipe[0], inbuf, 256);
        buff += string(inbuf);
        memset(inbuf, 0, 256);
        while (readReturn == 256)
        {
            readReturn = read(listenerPipe[0], inbuf, 256);
            buff += string(inbuf);
            memset(inbuf, 0, 256);
        }
        cout << "Read: " << buff << "\n";
        if (readReturn < 0)
        {
            perror(" Read from pipe \n");
            exit(1);
        }

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
                // Line represents a file
                addToQueue(socket, string(directory) + "/" + string(lineToken));
            }
            lineToken = strtok_r(NULL, "\n", &temp1);
            delete[] tempLineToken;
        }
        delete[] strTokBuff;
    }
}

void createDir(char *fileName)
{
    int dirSize;
    string directory = fileName;
    string dirAcum = "";
    dirSize = string(fileName).find_last_of("/");
    directory.resize(dirSize + 1);
    char *token = NULL;
    char *temp;
    char *fileNameBuf = new char[strlen(directory.c_str()) + 1];
    strcpy(fileNameBuf, directory.c_str());
    if ((directory.c_str())[0] == '.')
    {
        directory.erase(directory.begin());
    }
    cout << "Directory: " << directory << "\n";

    // Create the directory
    if (!dirExists((char *)(string("./tmp/")).c_str()))
    {
        mkdir((string("./tmp/")).c_str(), 0777);
        if (!dirExists((char *)(dirAcum).c_str()))
        {
            if (mkdir((string("./tmp/")).c_str(), 0777) < 0)
            {
                perror_exit("mkdir");
            }
        }
    }
    mkdir((string("./tmp/")).c_str(), 0777);
    dirAcum += "./tmp";
    // Create subdirs

    token = strtok_r(fileNameBuf, "/", &temp);
    while (token != NULL)
    {
        cout << "Subfolder: " << token << "\n";
        dirAcum += "/" + string(token);
        cout << "Acum: " << dirAcum << "\n";
        if (!dirExists((char *)(dirAcum).c_str()))
        {
            if (mkdir((dirAcum).c_str(), 0777) < 0)
            {
                perror_exit("mkdir");
            }
        }

        token = strtok_r(NULL, "/", &temp);
    }

    while (!dirExists((char *)(string("./tmp/") + directory).c_str()))
        ;

    cout << "Directory Created\n";
    // If file exists, delete
    if (dirExists((char *)(string("./tmp/") + string(fileName)).c_str()))
    {
        cout << "Directory Exists: " << fileName << "\n";
    }
    delete[] fileNameBuf;
}

void copyFile(char *fileName, int socket)
{
    char buf[blockSize + 1];
    string fileContents = "";
    int dirSize, pid;
    cout << "Filename: " << fileName << "\n";

    do
    {
        memset(buf, 0, blockSize + 1);
        if (read(socket, buf, blockSize) < 0)
            perror_exit("read");
        if (strcmp(buf, "EOF\n") != 0)
        {
            fileContents = fileContents + string(buf);
        }

    } while (strcmp(buf, "EOF\n") != 0);

    createDir(fileName);
}