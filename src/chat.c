#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

#include "../include/chat.h"
#include "../include/server_functions.h"
#include "../include/client_functions.h"

#define MSG_LEN 128

/* will handle all connections (request, response, prompts) */
void handle_conn(int sockfd, const char* node)
{
    char buff[MSG_LEN];

    // server starts with read
    if (strcmp(node, "server") == 0)
    {
        // read the message from client
        while (true)
        {
            bzero(buff, sizeof(buff));

            // read sockfd message into buff
            read(sockfd, buff, sizeof(buff));

            
            printf("CLIENT:: %s\n", buff);
            server_handler(buff, sockfd);
            bzero(buff, sizeof(buff));
        }
    }
    if (strcmp(node, "client") == 0)
    {
        // write a message to the server
        while (true)
        {
            bzero(buff, sizeof(buff));

            printf("> ");
            int n = 0;
            while((buff[n++] = getchar()) != '\n')
                ;            

            // write(sockfd, buff, sizeof(buff));
            buff[strlen(buff)-1] = '\0';
            
            if (strcmp(buff, "LOGIN") == 0)
            {
                // fill in authentication details and send login request to the server
                client_login(sockfd);
            }
            else
            {
                client_handler(buff, sockfd);
            }
        }
    }
}
