#ifndef UTILS
#define UTILS
#include <iostream>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <iterator>
#include <list>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include "jobScheduler.hpp"
#include "socketLockList.hpp"

class jobQueue;
class jobScheduler;
class socketLockList;

void child_server(int newsock);
void perror_exit(char *message);
void sigchld_handler(int sig);
void* commThread(void * arg);
bool dirExists(char *dir);
void addToQueue(int socket, std::string filename);
void getDirStructure(char *dir, int socket);
void copyFile(char* buf, int socket);
void createDir(char* fileName);


extern pthread_mutex_t queueLock;
extern pthread_mutex_t socketListLock;
extern pthread_cond_t queueEmptyCond;
extern pthread_cond_t queueFullCond;
extern jobQueue *queue;
extern socketLockList *sockets;
extern bool globalExit;
extern jobScheduler* pool;
extern int blockSize;
extern int queueSize;
extern int queueLimit;
#endif // UTILS