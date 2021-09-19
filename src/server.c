#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../include/server.h"
#include "../include/chat.h"

#define PORT 5050
#define MAX_POOL 5

/* initialize server socket and address and bind them */
int start_server()
{
    int server_sockfd;
    struct sockaddr_in serv_address;
    // create the server socket
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd == -1)
    {
        printf("failed to create socket...\n");
        exit(0);
    }
    
    // create the server address
    serv_address.sin_family = AF_INET;
    serv_address.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_address.sin_port = htons(PORT);

    // bind socket and address to create a complete communication endpoint

    int bind_status = bind(server_sockfd, (struct sockaddr *)&serv_address, sizeof(serv_address));
    if (bind_status != 0)
    {
        perror("bind error");
        printf("error binding socket to address...\n");
        exit(0);
    }
    return server_sockfd;
}

/* listen and handle incoming connections from clients (then close) */ 
void run_server(int server_sockfd)
{
    // enter a listening state
    if (listen(server_sockfd, MAX_POOL) != 0)
    {
        printf("failed to enter the listening state...\n");
        exit(0);
    }
    else
    {
        printf("[REMOTE BACKUP SERVER][Listening....]\n");
    }
    
    // accept and process client connections

    struct sockaddr_in client_address;
    socklen_t len = sizeof(client_address);
    int conn_fd = accept(server_sockfd, (struct sockaddr *)&client_address, &len);
    if (conn_fd == -1)
    {
        printf("error accepting connection from peer...\n");
        exit(EXIT_FAILURE);
    }

    handle_conn(conn_fd, "server");  // infinite loop will be here

    // close the socket
    close(server_sockfd);
}

// int main()
// {
//     // start and run the server
//     start_server();
//     run_server();

//     return 0;
// }