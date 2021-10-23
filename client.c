#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define LocalPort 8082
int roomPort = 0;
int turns[3][3] = {
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1}};

int connectServer(int port)
{
    int fd;
    struct sockaddr_in server_address;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    { // checking for errors
        printf("Error in connecting to server\nTry another port\n");
        return -1;
    }

    return fd;
}

void QRoom(int t1, int t2, int t3)
{
    int sock, broadcast = 1, opt = 1;
    char buffer[1024] = {0};
    struct sockaddr_in bc_address;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET;
    bc_address.sin_port = htons(roomPort);
    bc_address.sin_addr.s_addr = inet_addr("192.168.1.255");

    bind(sock, (struct sockaddr *)&bc_address, sizeof(bc_address));

    while (1)
    {
        memset(buffer, 0, 1024);
        for (int i = 0; i < 3; i++)
        { //for first q
            for (int j = 0; j < 3; j++)
            { // ask
                for (int z = 0; z < 3; z++)
                { //answer
                    if (turns[j][z] == 1)
                    {
                        printf("turn for %d of list:\n", z + 1);
                        read(0, buffer, 1024);
                        int a = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&bc_address, sizeof(bc_address));
                    }
                    if (turns[j][z] == 0)
                    {
                        recv(sock, buffer, 1024, 0);
                        printf("%s\n", buffer);
                    }
                }
            }
        }
    }
}

int main(int argc, char const *argv[])
{
    int fd;
    char buff[1024] = {0};

    if (argv[1] > 0)
    {
        fd = connectServer(atoi(argv[1]));
        printf("Port = %s\n for %d", argv[1], fd);
    }
    else
        fd = connectServer(LocalPort);

    if (fd < 0)
        return 0;

    recv(fd, buff, 1024, 0);
    printf("Server said: %s\n", buff);
    memset(buff, 0, 1024);
    int turn1, turn2, turn3;
    while (1)
    {
        read(0, buff, 1024);
        send(fd, buff, strlen(buff), 0);
        printf("Wait for Server response!\n");
        recv(fd, buff, 1024, 0);
        roomPort = atoi(&buff[0]);
        if (roomPort > LocalPort)
        {
            turn1 = atoi(&buff[4]);
            turn2 = atoi(&buff[6]);
            turn3 = atoi(&buff[8]);
            printf("Server said: on Port %d: Welcome to your question room! \n ", roomPort);
            break;
        }
        else
        {
            printf("Server said: %s\n", buff);
        }
        memset(buff, 0, 1024);
    }
    if (roomPort > 0)
    {
        printf("Turns: %d %d %d! \n ", turn1, turn2, turn3);
        QRoom(turn1, turn2, turn3);
    }

    return 0;
}