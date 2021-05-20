#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_SIZE 128000

int main(int argc, char *argv[])
{

    int sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1)
    {
        printf("Error creating socket!");
    }

    // struct for address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8008);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // connect
    int conn = connect(sock, (struct sockaddr *)&server_address, sizeof(server_address));

    if (conn == -1)
    {
        printf("Connection problem.\n\n");
        exit(0);
    }
    else
    {
        // check the passed args
        if (argc < 2)
        {
            printf("Expected command line arguments.\n");
            exit(0);
        }
        else
        {
            // check to send a login request to the server first
            if (strcmp(argv[1], "LOGIN") == 0)
            {
                char command[128];
                char *username = argv[2];
                char *password = argv[3];
                strcat(command, argv[1]);
                strcat(command, " ");
                strcat(command, username);
                strcat(command, " ");
                strcat(command, password);
                int s = send(sock, command, sizeof(command), 0);
                if (s < 1)
                {
                    printf("Error sending command!\n");
                }
            }
	    // accept the show commands command
            if(strcmp(argv[1], "SHOW COMMANDS") == 0){
		char command[128] = "SHOW COMMANDS";
		int s = send(sock, command, sizeof(command), 0);
		if (s < 1){
		    printf("Error sending command!\n");
		}
            }
            // accept the ls command
            if (strcmp(argv[1], "LS") == 0)
            {
                char command[4] = "LS";
                int s = send(sock, command, sizeof(command), 0);
                if (s < 1)
                {
                    printf("Error sending command!\n");
                }
            }

            // accept the LOCALCD command
            if (strcmp(argv[1], "LOCALCD") == 0)
            {
                char command[128];
                strcpy(command, "cd ");
                strcat(command, argv[2]);
                int ret = system(command);
                if (ret < 0)
                {
                    perror("program");
                }
            }

            // accept the LOCALLS command
            if (strcmp(argv[1], "LOCALLS") == 0)
            {
                system("stat -c \"\%n \%s \%y\" * ");
            }

            // accept remote CD command
            if (strcmp(argv[1], "CD") == 0)
            {
                char command[4] = "CD";
                int s = send(sock, command, sizeof(command), 0);
                if (s < 1)
                {
                    printf("Error sending command!\n");
                }                
            }

            // accept remote MKDIR command
            if (strcmp(argv[1], "MKDIR") == 0)
            {
                char command[8] = "MKDIR";
                int s = send(sock, command, sizeof(command), 0);
                if (s < 1)
                {
                    printf("Error sending command!\n");
                }                
            }

            if (strcmp(argv[1], "PUSH") == 0 || strcmp(argv[1], "PULL") == 0)
            {
                char *filename = argv[2];
                struct stat sb;
                unsigned long file_size;

                if (stat(filename, &sb) != -1)
                {
                    file_size = sb.st_size;
                    char filesize[100];
                    snprintf(filesize, 12, "%ld", file_size);

                    // this buffer will hold the contents read from the file
                    char *buff = (char *)calloc(file_size, sizeof(char));

                    if (strcmp(argv[1], "PUSH") == 0)
                    {
                        // read the contents of the file and send them too
                        int fd = open(filename, O_RDONLY, 0);
                        read(fd, buff, file_size);

                        // construct a command to send to server
                        char command[128] = "PUSH ";
                        strcat(command, strcat(filename, " "));
                        printf("CLIENT: %s\n", command);
                        strcat(command, filesize);
                        strcat(command, "|");
                        strcat(command, buff);
                        send(sock, command, sizeof(command), 0);

                        // send the command
                        int s = send(sock, command, sizeof(command), 0);
                        if (s < 1)
                        {
                            printf("Error sending command!\n");
                        }

                        // sednd the buffer containing the contents of the file
                        // send(sock, &buff, sizeof(buff), 0);
                    }
                    else if (strcmp(argv[1], "PULL") == 0)
                    {
                        // construct a command to send to server
                        char command[128] = "PULL ";
                        strcat(command, filename);

                        // send the PULL command to the server
                        send(sock, command, sizeof(command), 0);
                        printf("CLIENT: %s\n", command);
                        int s = send(sock, command, sizeof(command), 0);
                        if (s < 1)
                        {
                            printf("Error sending command!\n");
                        }
                    }
                    else
                    {
                        printf("Invalid command line arguments.\n");
                        exit(0);
                    }
                }
                else
                {
                    exit(0);
                }
            }
            if (strcmp(argv[1], "QUIT") == 0)
            { //  if (argv[1] == "QUIT")
                char command[128] = "QUIT";
                send(sock, command, sizeof(command), 0);
                printf("CLIENT: QUIT\n");
                close(sock);
            }
        }
    }

    char server_response[MAX_SIZE];
    recv(sock, &server_response, sizeof(server_response), 0);
    printf("SERVER: \n%s\n\n", server_response);

    return 0;
}
