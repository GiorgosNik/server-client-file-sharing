# server-client-datasharing

## Description
This project consists of two main components, the server and the client(s). The clients connect to the server and request the transfer of a directory and any included files from the server. The server receives the requests of the clients, stores the information of each file in a queue and sends them one by one using a thread-pool. The operation of these two programs, supporting utilities and structures, as well as compilation and execution instructions will be provided below.

## The Server
### Input and initialization
The server receives the following as command line arguments:
- the port to listen to 
- the size of the queue that will store the names of files awaiting transfer
- the size of the thread pool that will be used for sending the files
- the size of the block the files will be sent by

If the above are not provided in the correct format, an instruction message is printed. The server creates a thread pool of the size it received. This is implemented via the jobScheduler, a class that stores the ids of the worker threads and information pertaining to the parallelization of the file transfer task.

### Connecting and requesting a file
After the server has received the above, and finished setting up, it starts listening at the designated port. Every new connection request is accepted and the information is passed on to a specialized communication thread which then awaits for input, in the form of the directory to be transferred. The communication thread then replies with the size of the blocks the files it will send will be broken up in. The communication thread then examines the requested directory by creating a fork and executing "ls - R" to get all the contents, including sub directories and their content. For each file found, its name and location is added to the queue, a global structure that will be accessed by the worker threads later, in order to send the files. If the queue is full, the communication thread waits for a slot to become available. While the communication thread is performing this task, the main server is still waiting for and processing new connections. The information regarding these communication threads is stored in a list called pidList. This information is used to join these threads when the server terminates.

### Sending the files
The pool of worker threads, created at startup, is used to send the files requested by the client. The worker threads check the queue for files awaiting transfer. If the queue is empty, the workers wait for files to become available. After obtaining a file, the worker removes it from the queue and sends its name to the socket of the client that requested it. The worker then reads and sends the contents of the file, block by block, using the block size the server received as a command line argument. After sending the whole file, the worker checks if any other file transfers are pending for that particular client. If no such files are waiting in the queue or are being currently transferred, the worker terminates the connection and starts working on the next file, if it exists.

### Termination
The server runs continuously, until it receives a SIG INT signal. Upon receiving the signal, the server:
- Sets the exit flag and signals all threads waiting on the queue_empty and queue_full conditions to continue. 
- The server waits for all worker and comm threads to exit, using the preset pthread_t array and the pidList respectively.
- The global structures, like the queue and the pid list are deleted
- The jobScheduler is deleted

After the above, the server terminates

## The Client
### Input and initialization
The client receives the following as command line arguments:
- the port it will use to communicate with the server 
- the address of the server
- the directory to transfer

If the above are not provided in the correct format, an instruction message is printed.

### Receiving and copying the directory
After connecting to the server, the client sends the name of the directory to be received. The server (specifically the communication thread) replies with the size of the blocks the files will be sent in. After that, the client receives the directory/name combination of a file under the requested directory. The client then creates the directory structure under the local directory./tmp, if it does not already exist. After the directory structure has been copied, the client creates the specified file, opens it and writes the content it receives from the server. After receiving the "END" message, the client closes the file, and awaits for a new file name, to begin the process again. After receiving the message "EOF" instead of a file name, the client considers the transfer to be complete, and exits.

## Threads and Synchronization
The server makes extensive use of threads, in the form of worker and communication threads. These threads operate on some global structures, and in order to avoid data races, the program makes use of mutexes and condition variables. All global structures, eg. the queue and the list of pids of the workers, are protected by mutexes. In order to avoid busy waitin in case the queue is emtpy or full, the condition variables are used, transfering control to other threads until the requested resource becomes availiable. In order to avoid the phenomenon of two worker threads sending information to the same client at the same time, each client socket is paired with a mutex. When a worker has to write to a specific client, it requests the matching mutex be lowered, thus preventing two workers writing together.

## Setup, Execution
To create the executables, use the provided MAKEFILE like so:

 `make all`

 To remove the object files:

 `make clean`

 To run the dataServer executable:

 `./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>`

  To run the remoteClient executable:

 `./remoteClient -i <server_ip> -p <server_port> -d <directory>`

 The arguments for both the remoteClient and the dataServer can be provided in any order.