#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/server_functions.h"
#include "../include/generics.h"

/* handle server input *___input and proceed to process it */
void server_handler(char* input, int client_sockfd)
{
    // printf(">>> %s <<< && %d\n", command, strcmp(command, "LOGIN"));
    char* command = get_command(input);
    char message[MAX_MSG_LEN];

    if (strcmp(command, "RMKDIR") == 0)
    {
        // attempt mkdir on client machine
        char* dirname = get_filename(input);
        int mkdir_result = backup_mkdir(dirname);
        
        if (mkdir_result == -1)
        {
            serv_response(client_sockfd, "[server handler] remote mkdir failed...\n");
        }
        else if (mkdir_result == 1)
        {
            sprintf(message, "%s%s", "directory ", dirname);
            strcat(message, " exists.");
            serv_response(client_sockfd, message);
        }
        else
        {
            sprintf(message, "%s%s", "directory ", dirname);
            strcat(message, " created.");
            serv_response(client_sockfd, message);
        }
        free(command);
    }
    else if (strcmp(command,"RCD") == 0)
    {
        // attempt cd in client machine
        char* dirname = get_filename(input);
        if (backup_cd(dirname) == -1)
        {
            serv_response(client_sockfd, "[server handler] remote cd failed...\n");
        }
        else
        {
            sprintf(message, "%s%s", "directory changed to ", dirname);
            serv_response(client_sockfd, message);
        }        
        free(command);
    }
    else if (strcmp(command, "RLS") == 0)
    {
        // attempt ls in server side and send results to client machine
        remote_ls(client_sockfd);
        free(command);
    }
    else
    {
        free(command);
        server_recv(client_sockfd, input);
    }   
}

