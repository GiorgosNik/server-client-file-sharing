#include "utils.hpp"

using namespace std;

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
    char buf[4096 + 1];
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
    cout << "Client's parameters are:\n";
    cout << "Port: " << port << "\nServer Address: " << serverIp << "\nDirectory to copy " << directory << "\n";
    char directoryArray[directory.size() + 1];
    strcpy(directoryArray, directory.c_str());

    /* Create socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror_exit((char *)(string("socket")).c_str());
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
        perror_exit((char *)(string("connect")).c_str());
    }

    cout << "Connecting to " << serverIp << " port " << port << "\n";
    if (write(sock, directoryArray, 4096) < 0)
        perror_exit((char *)(string("write")).c_str());

    // Get the block size of the server
    if (read(sock, buf, 256) < 0)
        perror_exit((char *)(string("read")).c_str());
    blockSize = atoi(buf);
    cout << "Received Server Block Size: " << blockSize << "\n";

    // For each file, receive its name and copy it using copyFile()
    do
    {
        if (read(sock, buf, 4096) < 0)
            perror_exit((char *)(string("read")).c_str());
        if (strcmp(buf, "END\n") != 0)
        {
            copyFile(buf, sock);
        }

    } while (strcmp(buf, "END\n") != 0);
    close(sock);

    return 0;
}
