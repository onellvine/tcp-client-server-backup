#ifndef _GENERICS_H
#define _GENERICS_H

#define MAX_COMMAND_LEN 512
#define MAX_MSG_LEN 128

#define MAX_USERNAME_LEN 32
#define MAX_PASSWORD_LEN 64

#define MAX_FILE_SIZE 1000000

#define MAX_DIRNAME_LEN 128

#define MAX_LS_SIZE 512

#define MAX_CRED_SIZE 1024
#define MAX_TOK_SIZE 512
#define MAX_PARAM_SIZE 128


char* get_command(char* input);
char* get_filename(char* input);
char* get_credentials(char* filename, char* credential);
char* get_username(char* command);
char* get_password(const char *command);


char* compute_md5(const char* password, int length);
void get_pass(char* password, char* prompt);

void send_file(int sockfd, char* filename);
void recv_file(int sockfd, char* filename);

void serv_response(int sockfd, char* response);

void consume_buffer(int sockfd, char* buff);

int backup_mkdir(char* dirname);
int backup_cd(char* dirname);

void local_ls(void);

void remote_ls(int sockfd);

#endif