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

int setupServer(int port)
{
    struct sockaddr_in address;
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0); //TCP

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        printf("Error in binding\n");
        return -1;
    }
    if (listen(server_fd, 4) < 0)
    {
        printf("Error in listening\n");
        return -1;
    }

    return server_fd;
}

int acceptClient(int server_fd)
{
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t *)&address_len);

    return client_fd;
}

int numClient[4][3] = {0};
int numOnline[4] = {0};
int availablePort = LocalPort + 1;

int checkClients(char buffer[], int clientFD, int major)
{
    if (major > 0 && major < 5)
    {
        printf("client %d major is %d \n", clientFD, major);
        numClient[major - 1][numOnline[major - 1]] = clientFD;
        numOnline[major - 1] += 1;
        if (numOnline[major - 1] == 3)
        {
            printf("!!!Clients %d %d %d have same major!New port generated\n", numClient[major - 1][0], numClient[major - 1][1], numClient[major - 1][2]);
            numOnline[major - 1] = 0;
            sprintf(buffer, "%d %d %d %d", availablePort, numClient[major - 1][0], numClient[major - 1][1], numClient[major - 1][2]);
            availablePort += 1;
            return 1;
        }
    }
    else
    {
        sprintf(buffer, "You send wrong major id \n");
        send(clientFD, buffer, strlen(buffer), 0);
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, max_sd;
    char buffer[1024] = {0};
    fd_set master_set, working_set;

    if (argv[1] > 0)
    {
        server_fd = setupServer(atoi(argv[1]));
        printf("Port = %s\n ", argv[1]);
    }
    else
        server_fd = setupServer(LocalPort);

    if (server_fd < 0)
        return 0;

    FD_ZERO(&master_set);
    max_sd = server_fd;
    FD_SET(server_fd, &master_set);

    write(1, "Server is running \n\n\n", 23);

    int basePort = LocalPort + 1;

    while (1)
    {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);
        for (int i = 0; i <= max_sd; i++)
        {
            if (FD_ISSET(i, &working_set))
            {
                if (i == server_fd)
                { // new clinet
                    new_socket = acceptClient(server_fd);
                    FD_SET(new_socket, &master_set);
                    if (new_socket > max_sd)
                        max_sd = new_socket;
                    printf("New client connected. id = %d\n", new_socket);
                    sprintf(buffer, "choose: 1-Computer E. 2-Electrical E. 3-Civil E. 4-Mechanical E. \n");
                    send(new_socket, buffer, strlen(buffer), 0);
                    memset(buffer, 0, 1024);
                }
                else
                { // client sending msg
                    int bytes_received;
                    bytes_received = recv(i, buffer, 1024, 0);

                    if (bytes_received == 0)
                    { // EOF
                        printf("client fd = %d closed\n", i);
                        close(i);
                        FD_CLR(i, &master_set);
                        continue;
                    }
                    int major = atoi(&buffer[0]);
                    if (checkClients(buffer, i, major) == 1)
                    {
                        for (int i = 0; i < 3; i++)
                        {
                            send(numClient[major - 1][i], buffer, strlen(buffer), 0);
                        }
                        printf("Success!\n");
                    }
                    memset(buffer, 0, 1024);
                }
            }
        }
    }

    return 0;
}

//sprintf(buff,"%d",port)
//send(socket,strlen(msg))