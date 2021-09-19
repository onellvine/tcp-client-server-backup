#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/socket.h>

#include "../include/generics.h"

/* will send login command, with username and password to server*/
void client_login(int sockfd)
{
    char username[MAX_USERNAME_LEN];
    printf("username: ");
    fgets(username, MAX_USERNAME_LEN, stdin);
    username[strlen(username)-1] = '\0';

    char password[MAX_PASSWORD_LEN];
    get_pass(password, "password");  // generic function to get password from terminal
    char* password_hash = compute_md5(password, strlen(password)); // hashing the password (generic function)
    
    char command[MAX_COMMAND_LEN];
    strcat(command, "LOGIN ");
    strcat(command, username);
    strcat(command, " ");
    strcat(command, password_hash);

    if (send(sockfd, command, MAX_MSG_LEN, 0) == -1)
    {
        perror("[client login::send]");
        exit(EXIT_FAILURE);
    }

    char buff[MAX_MSG_LEN];
    consume_buffer(sockfd, buff);
}
