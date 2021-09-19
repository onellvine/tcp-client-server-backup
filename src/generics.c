#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <openssl/md5.h>

#include "../include/generics.h"

/* get a single command [eg PUSH] from *___input passed */
char* get_command(char* input)
{
    char* command = (char *)malloc(MAX_COMMAND_LEN);

    int i = 0;
    while (input[i] != ' ')
    {
        command[i] = input[i];
        i++;
    }

    return command;
}

/* get the filename from *___input passed */
char* get_filename(char* input)
{
    char* filename;

    while (*input != ' ')
    {
        input++; // increment the pointer address 
        if (*input == ' ')
        {
            input += 1; // skip the space
            filename = input; // set pointer address to begining of filename
            break;
        }
    }

    return filename;
}

/* get *___credentials from file *___filename; will often be used by the server */
char* get_credentials(char* filename, char* credential)
{
    size_t size = 0;

    FILE* fp = fopen(filename, "r");
    if (!fp)
    {
        printf("error opening credentials file...\n");
        exit(EXIT_FAILURE);
    }

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);

    char* credentials = (char *)malloc(size);
    if (!credentials)
    {
        printf("memory error reading credentials...\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    if (fread(credentials, 1, size, fp) != size)
    {
        printf("error reading credentials file...\n");
        free(credentials);
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    
    fclose(fp);

    // copy credentials to a new address and free the heap
    char credentials_copy[MAX_CRED_SIZE];
    strcpy(credentials_copy, credentials);
    free(credentials);

    // allocate token in the heap and assign through strtok
    char* token = (char *)calloc(MAX_TOK_SIZE, sizeof(char));

    char* chunk = strtok(credentials_copy, ":");
    strcpy(token, chunk);
    
    if (strcmp(credential, "username") == 0)
    {
        return token;
    }
    else if (strcmp(credential, "password") == 0)
    {
        token = strtok(NULL, ":");
        return token;
    }
    else if (strcmp(credential, "path") == 0)
    {
        char* password = token;
        while(token != NULL)
        {
            password = token;
            token = strtok(NULL, ":");
        }
        free(token);
        return password;
    }
    else{
        return NULL;
    }    
}

/* get *___username from the passed command */
char* get_username(char* command)
{
    char command_copy[MAX_PARAM_SIZE];
    strcpy(command_copy, command);  // because I want to tokenize
    char* username = (char *)calloc(MAX_TOK_SIZE, sizeof(char));
    
    // tokenize once => will return first word in command 
    char* chunk = strtok(command_copy, " ");

    //tokenize again to get the second variable (username)
    chunk = strtok(NULL, " ");

    // copy the chunk into username
    strcpy(username, chunk);

    return username;

}

/* get *___password from the passed command */
char* get_password(const char *command)
{
    char command_copy[MAX_PARAM_SIZE];
    strcpy(command_copy, command);
    char* password = (char *)calloc(32, sizeof(char));  // return this

    char* token = strtok(command_copy, " ");

    int i = 0;
    while(token[i] != '\0')
    {
        password[i] = token[i]; // attempt to retain the address of password
        i++;
    }

    while(token != NULL)
    {   
        i = 0;
        while(token[i] != '\0')
        {
            password[i] = token[i]; // attempt to retain the address of password
            i++;
        }
        token = strtok(NULL, " ");       
    }
    return password;
}


/* receive a plain text password and length, and hash it using md5 */
char* compute_md5(const char* password, int length)
{
    int n;
    MD5_CTX c;
    unsigned char digest[16];
    char* out = (char *)malloc(33);

    MD5_Init(&c);

    while (length > 0)
    {
        if (length > 512)
        {
            MD5_Update(&c, password, 512);
        }
        else
        {
            MD5_Update(&c, password, length);
        }
        length -= 512;
        password += 512;
    }

    MD5_Final(digest, &c);

    for(n = 0; n < 16; n++)
    {
        snprintf(&(out[n*2]), 3, "%02x", (unsigned int)digest[n]);
    }

    return out;
}

/* prompt and accept password without showing characters (supress echo) */
void get_pass(char* password, char* prompt)
{
    static struct termios old_term;
    static struct termios new_term;

    // get the settings of the actual terminal
    tcgetattr(STDIN_FILENO, &old_term);

    // supress echo (will not show characters typed)
    new_term = old_term;
    new_term.c_lflag &= ~(ECHO);

    // set the above suppression as new terminal attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

    // actual getting of the password,using a prompt
    printf("%s: ", prompt);
    if (fgets(password, MAX_PASSWORD_LEN, stdin) == NULL)
        password[0] = '\0';
    else
        password[strlen(password)-1] = '\0'; // replace return character

    printf("\n");
    // reset the terminal back to old settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
}

/* send a *___RESPONSE message to network socket ___SOCKFD, as a response */
void serv_response(int sockfd, char* response)
{
    // respond with appropriate message and continue to listen (or wait)
    if (send(sockfd, response, MAX_MSG_LEN, 0) == -1)
    {
        printf("[serv response] peer in darkness, send failed...\n");
        exit(EXIT_FAILURE);
    }
}

/* read *___FILENAME from directory and send the contents to ___SOCKFD */
void send_file(int sockfd, char* filename)
{
    struct stat file_stats;

    if(stat(filename, &file_stats) != 0)
    {
        perror("[send file]");
        printf("[send file] could not get the properties of %s\n", filename);
        serv_response(sockfd, "FILE STAT ERROR");
    }
    else
    {   
        char buff[file_stats.st_size];

        int fd = open(filename, O_RDONLY);
        if (fd == -1)
        {
            perror("[send file::open]");
            // serv_response(sockfd, "[send file] could not open file");
            exit(EXIT_FAILURE);
        }
        // read the contents of file fd
        int bytes = read(fd, buff, sizeof(buff));
        if (bytes == -1)
        {
            perror("[send file::read]");
            // serv_response(sockfd, "[send file] could not read file");
            exit(EXIT_FAILURE);
        }
        
        buff[file_stats.st_size] = '\0'; // null terminate the buffer
        printf("sending %d bytes of %s\n", bytes, filename);
        // send the contents of file fd to socket sockfd
        if (send(sockfd, buff, sizeof(buff), 0) == -1)
        {
            perror("[send file::send]");
            // serv_response(sockfd, "[send file] could not send file");
            exit(EXIT_FAILURE);
        }
    }
}

/* receive bytes sent by socket ___SOCKFD then create and store file *___FILENAME */
void recv_file(int sockfd, char* filename)
{
    char buff[MAX_FILE_SIZE];

    int fd = open(filename, O_WRONLY | O_CREAT);
    if (fd == -1)
    {
        perror("[recv file::open]");
        // serv_response(sockfd, "[recv file] could not open file");
        exit(EXIT_FAILURE);
    }

    // receive data from sockfd and write it to buff
    int bytes = recv(sockfd, buff, sizeof(buff), 0);
    if (bytes == -1)
    {
        perror("[recv file::recv]");
        // serv_response(sockfd, "[recv file] could not receive file");
        exit(EXIT_FAILURE);
    }

    // write the contents of buff to file fd
    printf("writing %d bytes to %s\n", bytes, filename);
    if (write(fd, buff, bytes) == -1)
    {
        perror("[recv file::write]");
        // serv_response(sockfd, "[recv file] could not write file");
        exit(EXIT_FAILURE);
    }
}

/* read the response sent by ___SOCKFD into *___BUFF and log it */
void consume_buffer(int sockfd, char* buff)
{    
    // read sockfd (server) message into buff
    if (read(sockfd, buff, MAX_MSG_LEN) == -1)
    {
        perror("[consume buffer::read]");
    };

    // print the server responses
    printf("SERVER:: %s\n", buff);

    if (strncmp(buff, "EXIT", 4) == 0)
    {
        close(sockfd);
        exit(EXIT_SUCCESS);  // will break outside the loop too
    }
}

/* make a directory identified by *___DIRNAME 
 * returns -1 on failure
*/
int backup_mkdir(char* dirname)
{
    struct stat dir_stats;
    if(stat(dirname, &dir_stats) == -1)
    {
        if (mkdir(dirname, 0777) != 0)  // mode_t with the right permissions
            return -1;
        else
            return 0;
    }
    return 1; // the directory already exists
}

/* change to the directory identified by *___DIRNAME
 * returns -1 on failure
 */
int backup_cd(char* dirname)
{
    if (chdir(dirname) != 0)
        return -1;
    else
        return 0;
}

/* list the contents of the current working directory */
void local_ls(void)
{
    system("ls");
}

/* execute ls in the current directory and send the results to ___SOCKFD */
void remote_ls(int sockfd)
{
    FILE *fp;
    char path[MAX_DIRNAME_LEN];
    char message[MAX_LS_SIZE];
    // clean the message buffer and prepare it for write
    bzero(message, sizeof(message));
    strcat(message, "\n");

    // open the command for reading
    fp = popen("/bin/ls .", "r");
    printf("reached here\n");
    if (fp == NULL) {
        serv_response(sockfd, "[remote ls] failed to run command ls..." );
    }

    // read the output and concatenate each path to message
    while (fgets(path, sizeof(path), fp) != NULL) {
        strcat(message, path);
    }

    // send ls results to sockfd
    if (send(sockfd, message, sizeof(message), 0) == -1) {
        printf("[remote ls] failed to send ls command...\n");
    }

    pclose(fp);
}

/* left here as a usage reference */

// int main()
// {
//     char* s_username = get_credentials("credentials.txt", "username");
//     printf("s_username: %s\n", s_username);
//     free(s_username);
//     char* s_password = get_credentials("credentials.txt", "password");
//     printf("s_password: %s\n", s_password);

//     char* command = "LOGIN kyle2046 5f4dcc3b5aa765d61d8327deb882cf99";
    
//     char* c_username = get_username(command);
//     printf("c_username: %s\n", c_username);
//     free(c_username);

//     char* c_password = get_password(command);
//     printf("c_password: %s\n", c_password);
//     free(c_password);

//     char* cred = get_credentials("credentials.txt", "path");
//     printf("cred: %s\n", cred);

//     return 0;
// }