#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

char credentials[3][128];
char client_cmd[8];
char user_data[3][128];

void  userData(char *client_msg)
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

int main()
{

    readCredentials();
    int i;
    char *username = credentials[0];
    printf("Username: %s\n", username);

    char *password = credentials[1];
    printf("Password; %s\n", password);

    char *directory = credentials[2];
    printf("Folder: %s\n", directory);

    char line[128] = "LOGIN kyle2046 password";

    getCommand(line);
    printf("command: %s\n", client_cmd);
    printf("Line %s\n", line);

    userData(line);
    
    char *uname = user_data[1];
    printf("Uname: %s\n", uname);

    char *paswd = user_data[2];
    printf("Paswd: %s\n", paswd);


    return 0;
}