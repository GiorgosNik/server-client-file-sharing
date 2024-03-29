#include "utils.hpp"
#include "jobQueue.hpp"
#include "jobScheduler.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    // Initialise the configuration with defualt values
    int port = -1;
    int threadPoolSize = -1;
    int queueSize = -1;
    int sock, newsock;
    string portArg("-p");
    string threadArg("-s");
    string queueArg("-q");
    string blockArg("-b");
    pthread_t id;
    struct sockaddr_in server, client;
    struct sockaddr *serverptr = (struct sockaddr *)&server;
    struct sockaddr *clientptr = (struct sockaddr *)&client;
    struct hostent *rem;
    socklen_t clientlen = sizeof(client);
    char buf[1];
    char testBuf[200];
    memset(testBuf, 0, 200);

    // Set Signal Handler to reap children
    signal(SIGINT, sigintHandler);

    // Set the arguments
    if (argc != 9)
    {
        // If the user provided bad input, give usage instructions
        cout << "Usage: ./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>\n";
        exit(23);
    }
    else
    {
        for (int i = 1; i < 9; i += 2)
        {
            if (portArg.compare(string(argv[i])) == 0)
            {
                if (port == -1)
                {
                    port = stoi(string(argv[i + 1]));
                }
                else
                {
                    cout << "Usage: ./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>\n";
                    exit(23);
                }
            }
            else if (threadArg.compare(string(argv[i])) == 0)
            {
                if (threadPoolSize == -1)
                {
                    threadPoolSize = stoi(string(argv[i + 1]));
                }
                else
                {
                    cout << "Usage: ./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>\n";
                    exit(23);
                }
            }
            else if (queueArg.compare(string(argv[i])) == 0)
            {
                if (queueSize == -1)
                {
                    queueSize = stoi(string(argv[i + 1]));
                }
                else
                {
                    cout << "Usage: ./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>\n";
                    exit(23);
                }
            }
            else if (blockArg.compare(string(argv[i])) == 0)
            {
                if (blockSize == -1)
                {
                    blockSize = stoi(string(argv[i + 1]));
                }
                else
                {
                    cout << "Usage: ./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>\n";
                    exit(23);
                }
            }
        }
    }
    cout << "Server's parameters are: ";
    cout << "Port: " << port << "\nThread Pool Size: " << threadPoolSize << "\nBlock Size: " << blockSize << "\nQueue Size: " << queueSize << "\n";

    queueLimit = queueSize;

    pool = new jobScheduler(threadPoolSize);

    /* Create socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror_exit((char *)(string("socket").c_str()));
    }

    server.sin_family = AF_INET; /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port); /* The given port */

    /* Bind socket to address */
    if (bind(sock, serverptr, sizeof(server)) < 0)
    {
        perror_exit((char *)(string("bind").c_str()));
    }

    /* Listen for connections */
    if (listen(sock, 5) < 0)
    {
        perror_exit((char *)(string("listen").c_str()));
    }

    cout << "Listening for connections to port " << port << "\n";

    while (1)
    {
        if ((newsock = accept(sock, clientptr, &clientlen)) < 0)
        {
            perror_exit((char *)(string("accept").c_str()));
        }
        cout << "Accepted connection\n";
        // Pass the connection to a new communication thread and keep listening
        pthread_create(&id, NULL, &commThread, (void *)(&newsock));
        if (commThreads == NULL)
        {
            commThreads = new pidList(id);
        }
        else
        {
            commThreads->addToList(id);
        }
    }
    return 0;
}
