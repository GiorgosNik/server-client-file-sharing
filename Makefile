SERVER	= dataServer
CLIENT	= remoteClient
CC	 = g++
FLAGS = -O3 -g -pthread -fPIC -std=c++17 -o
OFLAGS = -c -O3 -g -pthread -fPIC  -Wall -std=c++17 -o
OBJECTS = utils.o jobQueue.o jobScheduler.o socketLockList.o commThreadList.o 
SOURCE = utils.cpp manager.cpp worker.cpp 

all: dataServer.cpp remoteClient.cpp
	$(CC) $(OFLAGS) jobQueue.o jobQueue.cpp
	$(CC) $(OFLAGS) socketLockList.o socketLockList.cpp
	$(CC) $(OFLAGS) commThreadList.o commThreadList.cpp
	$(CC) $(OFLAGS) utils.o utils.cpp
	$(CC) $(OFLAGS) jobScheduler.o jobScheduler.cpp
	$(CC) $(FLAGS) $(SERVER) $(OBJECTS) dataServer.cpp
	$(CC) $(FLAGS) $(CLIENT) $(OBJECTS) remoteClient.cpp

clean: 
	rm -f $(OBJECTS)
