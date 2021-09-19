#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/server.h"
#include "../include/client.h"

int server_sockfd;
int client_sockfd;


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("[main]: expected command line arguments...\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(argv[1], "server") == 0)
    {
        server_sockfd = start_server();
        run_server(server_sockfd);
    }
    else if (strcmp(argv[1], "client") == 0)
    {
        client_sockfd = create_client();
        connect_server(client_sockfd);
    }
    else
    {
        printf("[main]: invalid command line argument...\n");
        exit(EXIT_FAILURE);
    }
    
    return 0;
}