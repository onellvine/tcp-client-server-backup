#ifndef _SERVER_FUNCTIONS_H
#define _SERVER_FUNCTIONS_H

void server_login(int sockfd, char* uname, char* paswd);

void server_recv(int client_sockfd, char* client_input);
void server_handler(char* command, int client_sockfd);

#endif