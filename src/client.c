#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../include/client.h"
#include "../include/chat.h"

#define PORT 5050


/* create a client socket to use in connections */
int create_client()
{
    int client_sockfd;
    
    client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sockfd == -1)
    {
        printf("error creating client socket...\n");
        exit(EXIT_FAILURE);
    }

    return client_sockfd;
}

/* establish a connection to the server (and send data) */
void connect_server(int client_sockfd)
{
    struct sockaddr_in serv_addr;

    // premeditate server ip and port (to connect to)
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(PORT);

    if (connect(client_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
    {
        printf("error connecting to server...\n");
        exit(0);
    }

    handle_conn(client_sockfd, "client");

    // finally close the client socket
    close(client_sockfd);

}

// int main()
// {
//     // create client and connect to server
//     create_client();
//     connect_server();
// }
