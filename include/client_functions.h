#ifndef _CLIENT_FUNCTIONS_H 
#define _CLIENT_FUNCTIONS_H

void client_login(int sockfd);
void client_send(int sockfd, char* input);
void client_handler(char* input, int server_sockfd);

#endif