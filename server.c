#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#define MAXBUFLEN 1000000
#define MAX_SIZE 128000

int getFileSize(char str[]);
char *getFileName(char *str);
char *getFileContents(char *str);
void getCommand(char *str);
void readCredentials();
void userData(char *client_msg);
void availableCommands();

char credentials[3][128];
char user_data[3][128];
char client_cmd[8];

int main(int argc, char *argv[])
{
    char server_msg[4] = "OK";
    int client_sock;

    // check for command line args
    if (argc < 2)
    {
        printf("Command line arguments needed\n");
        exit(0);
    }

    int loggedin = 0;

    // proceed to listen to connections and accept
    printf("Listening...\n");

    char client_msg[MAX_SIZE] = {0};
    char *p;

    int server_sock;
    server_sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address, client_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(strtol(argv[2], &p, 10));
    server_address.sin_addr.s_addr = INADDR_ANY;

    // bind the connection
    if (bind(server_sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("could not bind socket\n");
    }

    //listen to connections (loop)
    listen(server_sock, 5);

    while (1)
    {

        int clientLen = sizeof(struct sockaddr_in);
        client_sock = accept(server_sock, (struct sockaddr *)&client_address, (socklen_t *)&clientLen);
        if (client_sock < 0)
        {
            printf("Accpet failed!\n");
        }

        // receive the command from the client
        int r_command = recv(client_sock, &client_msg, sizeof(client_msg), 0);

        // begin by processing login command
        getCommand(client_msg);

        if (strcmp(client_cmd, "LOGIN") == 0)
        {
            printf("CLIENT: %s\n", client_msg);
            // call the function to read credentials from file
            readCredentials();
            char *username = credentials[0];
            char *password = credentials[1];
            char *user_path = credentials[2];

            // get user data (credentials) from client msg
            userData(client_msg);
            char *uname = user_data[1];
            char *paswd = user_data[2];

            if (strcmp(username, uname) == 0 && strcmp(password, paswd) == 0)
            {
                loggedin = 1; // login the user
                if (chdir(user_path) != 0)
                { // change directory to the users path
                    perror("Program -> chdir() Failed");
                }
                // respond to the client with OK
                send(client_sock, server_msg, sizeof(server_msg), 0);
            }
            else if (strcmp(password, paswd) != 0)
            {
                char error[128] = "Your password was incorrect!";
                send(client_sock, error, sizeof(error), 0);
            }
            else
            {
                char error[128] = "Invalid login credentials";
                send(client_sock, error, sizeof(error), 0);
            }
        }

        if (loggedin == 1)
        {
            listen(server_sock, 5);
            while (1)
            {
                client_sock = accept(server_sock, (struct sockaddr *)&client_address, (socklen_t *)&clientLen);
                if (client_sock < 0)
                {
                    printf("Accpet failed!\n");
                }

                int rcv = recv(client_sock, &client_msg, sizeof(client_msg), 0);
                char client_msg_cpy[MAX_SIZE];
                strcpy(client_msg_cpy, client_msg);
                // get the command sent by the client
                getCommand(client_msg_cpy);

                if (strcmp(client_cmd, "LS") == 0) // ls contents of the cwd
                {
                    printf("CLIENT: %s\n", client_cmd);
                    system("stat -c \"\%n \%s \%y\" * >> ls.txt");
                    char src[MAXBUFLEN + 1];
                    FILE *fp = fopen("ls.txt", "r");
                    if (fp != NULL)
                    {
                        size_t len = fread(src, sizeof(char), MAXBUFLEN, fp);
                        src[len++] = '\0';
                    }
                    send(client_sock, src, sizeof(src), 0);
                    char message[8] = "OK";
                    send(client_sock, message, sizeof(message), 0);
                    system("rm ls.txt"); // delete the file after using it
                }
                else if (strcmp(client_cmd, "MKDIR") == 0) // create a directory with the given name (argv[2])
                {
                    printf("CLIENT: %s\n", client_cmd);
                    char command[128];
                    strcpy(command, "mkdir ");
                    strcat(command, argv[2]);
                    system(command);
                    char message[8] = "OK";
                    send(client_sock, message, sizeof(message), 0);
                }
                else if (strcmp(client_cmd, "CD") == 0)
                {
                    printf("CLIENT: %s\n", client_cmd);
                    char command[128];
                    strcpy(command, "cd ");
                    strcat(command, argv[2]);
                    int ret = system(command);
                    if (ret < 0)
                    {
                        char message[16] = "Failed";
                        printf("%s\n\n", message);
                        send(client_sock, message, sizeof(message), 0);
                    }
                    else
                    {
                        char message[8] = "OK";
                        printf("%s\n\n", message);
                        send(client_sock, message, sizeof(message), 0);
                    }
                }
                else
                {
                    // process the client message and proceed to recev the file contents
                    int file_size = getFileSize(client_msg);

                    char full_path[260];
                    char filename[128] = {"/"};
                    char *file_name = getFileName(client_msg);
                    printf("Did I segment and fail?\n");
                    strcat(filename, file_name);

                    // ensure the file goes to the created directory
                    sprintf(full_path, "%s%s", argv[1], filename);

                    if (file_size != 0)
                    { // assumes that PUSH command was given
                        char *file_contents = getFileContents(client_msg_cpy);

                        printf("CLIENT: PUSH %s\n", file_name);
                        // make the directory for storing the backups
                        struct stat dir = {0};
                        if (!(stat(argv[1], &dir) == 0 && S_ISDIR(dir.st_mode)))
                        {
                            mkdir(argv[1], 0755);
                        }
                        // create and open the file to write the bytes
                        int fd = open(full_path, O_RDWR | O_CREAT);
                        if (fd == -1)
                        {
                            printf("Failed to create file %s\n", full_path);
                            perror("Program");
                        }
                        // write the received bytes of data
                        int written = write(fd, file_contents, file_size);
                        if (written < 0)
                        {
                            printf("Failed to save file!\n");
                            perror("Program");
                        }

                        // respond to the client with an "OK"
                        send(client_sock, server_msg, sizeof(server_msg), 0);
                    }
                    else
                    {
                        if (strcmp(client_msg, "PULL") == 0)
                        {
                            struct stat sb;
                            // retreive the file and send the bytes to the client
                            printf("CLIENT: PULL %s\n", file_name);
                            if (stat(full_path, &sb) != -1)
                            {
                                file_size = sb.st_size;
                            }

                            // this buffer will hold the bytes to send to the client
                            char *buff = (char *)calloc(file_size, sizeof(char));

                            // open the file for reading the bytes
                            int fd = open(full_path, O_RDWR);
                            if (fd == -1)
                            {
                                printf("Failed to open the requested file %s\n", full_path);
                                perror("Program");
                            }

                            // read bytes from the file into the buffer
                            int read_b = read(fd, buff, file_size);
                            if (read_b < 0)
                            {
                                printf("Error reading the requested file %s\n", full_path);
                                perror("Program");
                            }

                            // send the read bytes to the client
                            send(client_sock, buff, file_size, 0);

                            // respond to the client with an "OK" (different send())
                            send(client_sock, server_msg, sizeof(server_msg), 0);
                        }
                        else
                        {
                            printf("CLIENT: QUIT\n");
                            //close the connection and quit the program
                            // close(server_sock);
                            exit(0);
                        }
                    }
                }

                if (rcv < 0)
                {
                    printf("Error receiving packets\n");
                }
            }
        }
        else
        {
            char error[128] = "Will not process any commands. You arent logged in";
            send(client_sock, error, sizeof(error), 0);
        }
    }

    return 0;
}

void availableCommands(){
    printf("Available commands\n");
    printf("LOGIN username password\n");
    printf("MKDIR dirname\n");
    printf("CD dirname\n");
    printf("PUSH filename.ext\n");
    printf("PULL filename.txt\n");
    printf("QUIT\n");
    printf("\n");
}

int getFileSize(char str[])
{

    char num[128] = {0};

    size_t len = strlen(str) - 1;

    for (size_t i = 0; i < len; i++)
    {
        if (isdigit(str[i]))
        {
            strcat(num, (const char *)&str[i]);
            break;
        }
    }

    return atoi(num);
}

char *getFileName(char *str)
{

    char *token = strtok(str, " ");
    char *filename;

    if (strcmp(token, "QUIT") == 0)
    {
        filename = token;
        return filename;
    }

    while (token != NULL)
    {
        token = strtok(NULL, " ");
        if (strcmp(token, "PUSH") != 0 || strcmp(token, "PULL") != 0)
        {
            filename = token;
            break;
        }
    }

    return filename;
}

char *getFileContents(char *str)
{
    char *token = strtok(str, "|");
    char *file_contents;

    while (token != NULL)
    {
        token = strtok(NULL, "|");

        if (strcmp(token, "|") != 0)
        {
            file_contents = token;
            break;
        }
    }

    return file_contents;
}

void readCredentials()
{
    FILE *fp = fopen("credentials.txt", "r");

    const char s[2] = ":";
    char *token;
    int i = 0;
    if (fp != NULL)
    {
        char line[128];
        while (fgets(line, sizeof line, fp) != NULL)
        {
            token = strtok(line, s);
            while (token != NULL)
            {
                strcpy(credentials[i++], token);
                token = strtok(NULL, s);
            }
            printf("\n");
        }
        fclose(fp);
    }
    else
    {
        perror("credentials.txt");
    }
}

void getCommand(char *line)
{
    char copy[128];
    strcpy(copy, line);
    char *token;
    int i = 0;
    token = strtok(copy, " ");
    strcpy(client_cmd, token);
}

void userData(char *client_msg)
{
    char *token;
    char copy[128];
    strcpy(copy, client_msg);
    int i = 0;
    token = strtok(copy, " ");
    while (token != NULL)
    {
        strcpy(user_data[i++], token);
        token = strtok(NULL, " ");
    }
}
