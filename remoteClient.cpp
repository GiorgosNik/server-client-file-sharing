#include "utils.hpp"

using namespace std;
void child_server(int newsock);
void perror_exit(char *message);
void sigchld_handler(int sig);


int main(int argc, char *argv[])
{
    // Init the configuration with defualt values
    int port = -1;
    int sock, i;
    blockSize = -1;
    string serverIp("");
    string directory("");
    string portArg("-p");
    string ipArg("-i");
    string directoryArg("-d");
    char buf[256];
    char tempIpBuff[1000];
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr *)&server;
    struct hostent *rem;

    // Set the arguments
    if (argc != 7)
    {
        // If the user provided bad input, give usage instructions
        cout << "Usage: ./remoteClient -i <server_ip> -p <server_port> -d <directory>\n";
        exit(23);
    }
    else
    {
        for (int i = 1; i < 7; i += 2)
        {
            // Port Argument
            if (portArg.compare(string(argv[i])) == 0)
            {
                if (port == -1)
                {
                    port = stoi(string(argv[i + 1]));
                }
                else
                {
                    cout << "Usage: ./remoteClient -i <server_ip> -p <server_port> -d <directory>\n";
                    exit(23);
                }
            }
            if (ipArg.compare(string(argv[i])) == 0)
            {
                if (serverIp.compare(string("")) == 0)
                {
                    serverIp = string(argv[i + 1]);
                }
                else
                {
                    cout << "Usage: ./remoteClient -i <server_ip> -p <server_port> -d <directory>\n";
                    exit(23);
                }
            }
            if (directoryArg.compare(string(argv[i])) == 0)
            {
                if (directory.compare(string("")) == 0)
                {
                    directory = string(argv[i + 1]);
                }
                else
                {
                    cout << "Usage: ./remoteClient -i <server_ip> -p <server_port> -d <directory>\n";
                    exit(23);
                }
            }
        }
    }
    cout << "Port: " << port << " serverIp: " << serverIp << " directory " << directory << "\n";
    char directoryArray[directory.size() + 1]; // as 1 char space for null is also required
    strcpy(directoryArray, directory.c_str());

    /* Create socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror_exit("socket");
    }

    /* Find server address */
    strcpy(tempIpBuff, serverIp.c_str());
    if ((rem = gethostbyname(tempIpBuff)) == NULL)
    {
        herror("gethostbyname");
        exit(1);
    }
    server.sin_family = AF_INET;
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port);
    if (connect(sock, serverptr, sizeof(server)) < 0)
    {
        perror_exit("connect");
    }

    cout << "Connecting to " << serverIp << " port " << port << "\n";
    if (write(sock, directoryArray, 256) < 0)
        perror_exit("write");
    if (read(sock, buf, 256) < 0)
            perror_exit("read");  
    blockSize = atoi(buf);
    cout<<"BlockSize "<<blockSize<<"\n";
    do
    {
        if (read(sock, buf, 256) < 0)
            perror_exit("read");
        if (strcmp(buf, "END\n") != 0)
        {
            copyFile(buf,sock);
        }

    } while (strcmp(buf, "END\n") != 0);
    close(sock);

    return 0;
}

