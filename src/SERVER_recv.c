#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>

#include "../include/server_functions.h"

#include "../include/generics.h"

/* receives the client's command (from packets sent) and processes it */
/* command ___ PUSH, PULL [filename], EXIT */
void server_recv(int client_sockfd, char* client_input)
{
    // char* server_res = "OK";
    char* command = get_command(client_input);
    char* filename;

    if (strcmp(command, "PUSH") == 0)
    {
        filename = get_filename(client_input);
        recv_file(client_sockfd, filename); // will recv sent bytes and create/save the file
        serv_response(client_sockfd, "OK");
    }
    else if (strcmp(command, "PULL") == 0)
    {
        filename = get_filename(client_input);
        send_file(client_sockfd, filename); // will read file and send the bytes back
    }
    else if (strcmp(command, "EXIT") == 0)
    {
        serv_response(client_sockfd, "EXIT");
        free(command);
        exit(EXIT_SUCCESS);
    }
    else if (strcmp(command, "LOGIN") == 0)
    {
        // serv_response(client_sockfd, "OK");
        char* username = get_username(client_input);
        char* password = get_password(client_input);
        
        server_login(client_sockfd, username, password);
    }
    else
    {
        // server_res = "NOT AVAILABLE";
        serv_response(client_sockfd, "NOT AVAILABLE");
    }
    free(command);
}