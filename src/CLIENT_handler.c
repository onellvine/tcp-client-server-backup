#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/client_functions.h"
#include "../include/generics.h"

/* handle client input *__input and proceed to process it */
void client_handler(char* input, int server_sockfd)
{
    // printf(">>> %s <<< && %d\n", input, strcmp(input, "LOCALLS"));
    char* command = get_command(input);

    if (strcmp(command, "MKDIR") == 0)
    {
        // attempt mkdir on client machine
        char* dirname = get_filename(input);
        int mkdir_result = backup_mkdir(dirname);
        
        if (mkdir_result == -1)
            printf("[client handler] local mkdir failed...\n");
        else if (mkdir_result == 1)
            printf("directory %s exists.\n", dirname);
        else
            printf("directory %s created.\n", dirname);
        free(command);
    }
    else if (strcmp(command, "CD") == 0)
    {
        // attempt cd in client machine
        char* dirname = get_filename(input);
        if (backup_cd(dirname) == -1)
            printf("[client handler] local cd failed...\n");
        else
            printf("directory changed to %s\n", dirname);        
        free(command);
    }
    else if (strcmp(command, "LS") == 0)
    {
        // attempt ls in client machine
        local_ls();
        free(command);
    }
    else  // PUSH, PULL, EXIT, [RMKDIR, RCD, RLS]
    {
        free(command);
        client_send(server_sockfd, input);
    }
}
