#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>

#include "../include/client_functions.h"

#include "../include/generics.h"

/* receives the client's command (from input) and sends it to the server */
/* command ___ PUSH, PULL [filename], EXIT, RMKDIR, RCD, RLS*/
void client_send(int server_sockfd, char* input)
{
    if (send(server_sockfd, input, MAX_MSG_LEN, 0) == -1)
    {
        perror("[client send::send]");
        exit(EXIT_FAILURE);
    }
    
    char* command = get_command(input);
    char* filename = get_filename(input);
    char buff[MAX_MSG_LEN];

    if (strcmp(command, "PUSH") == 0)
    {
        // read the file and send the bytes to the server
        send_file(server_sockfd, filename);
        consume_buffer(server_sockfd, buff);
    }
    else if (strcmp(command, "PULL") == 0)
    {
        // wait for the server's response (will send file, or respond with error)
        recv_file(server_sockfd, filename);
    }
    else if (strcmp(command, "EXIT") == 0)
    {
        // wait for, and log the last server response and then close the socket
        free(command);
        consume_buffer(server_sockfd, buff);
    }
    else if (strcmp(command, "RMKDIR") == 0 || strcmp(command, "RCD") == 0 || strcmp(command, "RLS") == 0)
    {
        // wait for and log server response regarding file operations (or commands)
        consume_buffer(server_sockfd, buff);
    }
    else
    {
        printf("command not available...\n");
        consume_buffer(server_sockfd, buff);
        // continue to prompt for new command
    }    
    free(command);
}