#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/socket.h>

#include "../include/server_functions.h"

#include "../include/generics.h"

/* 
attempt chdir to dir to account identified by *___uname, then 
authenticate *___uname with *___paswd using stored credentials,
in the account dir and then send relevant response to ___SOCKFD 
*/
void server_login(int sockfd, char* uname, char* paswd)
{
    char accounts_dir[32] = "../res/accounts/";
    char* user_dir = strcat(accounts_dir, uname);
    
    if (chdir(user_dir) != 0)
    {
        serv_response(sockfd, "NO ACCOUNT");
    }
    else // compare uname and password to stored credentials
    {
        char* username = get_credentials("credentials.txt", "username");
        char* password = get_credentials("credentials.txt", "password");

        if (strcmp(uname, username) == 0 && strcmp(paswd, password) == 0)
        {
            serv_response(sockfd, "OK");
        }
        else if(strcmp(uname, username) == 0 && strcmp(paswd, password) != 0)
        {
            chdir("../../");
            serv_response(sockfd, "INCORRECT PASSWORD");
        }
        else
        {
            chdir("../../");
            serv_response(sockfd, "INVALID LOGIN CREDENTIALS");
        }
    }
}