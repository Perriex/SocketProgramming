#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>

#define LocalPort 8080
int roomPort = 0;

int connectServer(int port)
{
    int fd;
    struct sockaddr_in server_address;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("Error in connecting to server\nTry another port\n");
        return -1;
    }

    return fd;
}

struct Questions
{
    char ask[300];
    char answer[2][300];
    char bestAnswer[1];
} QT;
struct sockaddr_in bc_address;

void alarm_handler(int sig)
{
    printf("Times out!\n");
}

int ifPassed(char buffer[])
{
    if (buffer[0] == 'p' && buffer[1] == 'a' && buffer[2] == 's' && buffer[3] == 's' && buffer[4] == '\n')
    {
        return 1;
    }
    return 0;
}

void getTurn(int sock, int t1, int t2, int t3, int id)
{
    char buffer[1024] = {0};
    int turns[9][3] = {
        {t1, t2, t3},
        {t3, t1, t2},
        {t2, t3, t1},
        {t2, t3, t1},
        {t3, t1, t2},
        {t1, t2, t3},
        {t3, t1, t2},
        {t1, t2, t3},
        {t2, t3, t1}};
    signal(SIGALRM, alarm_handler);
    siginterrupt(SIGALRM, 1);
    int indexAnswer = 0;
    for (int j = 0; j < 9; j++)
    {
        if (turns[j][0] == id)
        {
            memset(buffer, 0, 1024);
            printf("Your Turn\n");
            int read_ret = 0;
            if (j % 3 == 0)
            {
                printf("Ask:\n");
                indexAnswer = 1;
                read_ret = read(0, buffer, 1024);
                strcpy(QT.ask, buffer);
            }
            else
            {
                printf("You have 60 seconds to answer the question:\nAnswer:\n");
                alarm(60);
                read_ret = read(0, buffer, 1024);
                alarm(0);
            }
            if (read_ret < 0)
            {
                sprintf(buffer, "~ Time is over for %d\n", id);
            }
            sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&bc_address, sizeof(bc_address));
            recv(sock, buffer, 1024, 0);
        }
        else
        {
            memset(buffer, 0, 1024);
            printf("Turn for others!\nWait...\n\n");
            recv(sock, buffer, 1024, 0);
            if (indexAnswer > 0 && indexAnswer < 3)
            {
                if (ifPassed(buffer) == 1 || buffer[0] == '~')
                {
                    strcpy(QT.answer[indexAnswer - 1], "-\n");
                }
                else
                {
                    strcpy(QT.answer[indexAnswer - 1], buffer);
                }
                indexAnswer += 1;
            }
            printf("%s\n", buffer);
        }
    }
}

void createAnswer()
{
    if (QT.answer[0] == "-\n")
        QT.bestAnswer[0] = '2';
    else if (QT.answer[1] == "-\n")
        QT.bestAnswer[0] = '1';
    else
        QT.bestAnswer[0] = rand() * 200 % 2 ? '1' : '2';
    strcat(QT.ask, "answers:\n");
    strcat(QT.ask, QT.answer[0]);
    strcat(QT.ask, QT.answer[1]);
    strcat(QT.ask, "best answer:\n");
    strcat(QT.ask, QT.bestAnswer);
}

int QRoom(int t1, int t2, int t3, int id)
{
    int sock, broadcast = 1, opt = 1;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET;
    bc_address.sin_port = htons(roomPort);
    bc_address.sin_addr.s_addr = inet_addr("192.168.1.255");

    bind(sock, (struct sockaddr *)&bc_address, sizeof(bc_address));
    getTurn(sock, t1, t2, t3, id);
    return sock;
}

int main(int argc, char const *argv[])
{
    int fd,sock;
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
    int id = atoi(&buff[0]);
    printf("Server said: Client %s\n", buff);
    memset(buff, 0, 1024);
    int turn1, turn2, turn3;
    int majorId;
    while (1)
    {
        read(0, buff, 1024);
        majorId = atoi(&buff[0]);
        send(fd, buff, strlen(buff), 0);
        printf("Wait for Server response!\n");
        recv(fd, buff, 1024, 0);
        roomPort = atoi(&buff[0]);
        if (roomPort > LocalPort)
        {
            turn1 = atoi(&buff[4]);
            turn2 = atoi(&buff[6]);
            turn3 = atoi(&buff[8]);
            printf("Port %d: Welcome to your question room! \n ", roomPort);
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
        printf("Clients in room: %d %d %d! \n ", turn1, turn2, turn3);
        sock = QRoom(turn1, turn2, turn3, id);
        createAnswer();
        sprintf(buff, "@%d\n%s\n____________________________\n", majorId, QT.ask);
        send(fd, buff, strlen(buff), 0);
        close(sock);
    }
    memset(buff, 0, 1024);
    recv(fd, buff, 1024, 0);
    printf("Server said: %s\n", buff);
    printf("Bye Bye!\n");
    close(fd);
    return 0;
}