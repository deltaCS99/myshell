#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

bool is_exited (char* str);
char* get_string();
char** args_arr(char* string);
int args_length(char** args);
int process_args(int args_length, char ** args);
int redirection(char** args);
int other_cmds(char ** args);
int print_error();
int args_to_path(int args_length, char** args);


char** PATH ;

int main(int argc, char* argv[])
{
    int status;

    char** path = malloc(6 * sizeof(char*));
    path[0] = "/bin/";

    PATH = path;

    printf("PATH :%s\n", PATH[0]);

    if ( argc == 1)
    {
        char* input;

        do
        {
            input = get_string();
            char** args = args_arr(input);
            int args_c = args_length(args);

            status = process_args(args_c, args);


        }
        while (status);
    }
    else if (argc == 2)
    {
        FILE* file = fopen(argv[1], "r");

        if (file == NULL)
        {
            print_error();
        }

        char *line;
        size_t buffersize = 32;
        size_t chars;

        line = (char *)malloc(buffersize * sizeof(char)-1);

        while (getline(&line, &buffersize, file) != -1)
        {
            int len = strlen(line);
            line[len-1] = '\0';
            printf("COMMAND : %s\n",line);
            char** args = args_arr(line);
            int args_c = args_length(args);
            process_args(args_c, args);
        }

    }
}

char* get_string()
{
    char *string;
    size_t buffersize = 32;
    size_t chars;

    string = (char *)malloc(buffersize * sizeof(char)-1);
    if (string == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }

    printf("witsshell> ");
    chars = getline(&string, &buffersize, stdin);
    string[chars-1] = '\0';

    return string;
}

char** args_arr(char* string)
{
    int i = 0;

    char **words = malloc(20 * sizeof(char*));

    char *token;

    token = strsep(&string," ");

    while ( token != NULL)
    {
		words[i] = token;
        i++;
        token = strsep(&string, " ");
    }

    return words;
}

int args_length(char** args)
{
    int length = 0;

    while (args[length] != NULL)
    {
        length++;
    }

    return length;
}

int process_args(int args_length, char ** args)
{

    if( strcmp(args[0], "exit") == 0)
    {
        exit(0);
    }

    else if (strcmp(args[0], "cd") == 0)
    {
        if(args[1] == NULL)
        {
            print_error();
        }
        else
        {
            if(chdir(args[1]) == 0)
            {
                char* cwd = getcwd(NULL, 0);
                printf("cwd: %s\n", cwd);
                return 1;
            }
            else
            {
                print_error();
            }
        }
    }
    else if (strcmp(args[0], "path") == 0)
    {
        if(args[1] == NULL)
        {
            print_error();
        }
        else
        {
            args_to_path(args_length, args);
        }
    }
    else
    {
        other_cmds(args);
    }

    return 1;
}


int other_cmds(char ** args)
{
    int length = args_length(PATH);
    int args_len = args_length(args);
    char* arg0 = args[0];

    for (int i = 0 ; i < length ; i++)
    {
        char* dest = malloc(40 * sizeof(char*));
        strcpy(dest, PATH[i]);
        strcat(dest, arg0);
        int status = access(dest, X_OK);

        if(status == 0)
        {
            int pid = fork();

            if (pid == 0)
            {
                int redir_pos =  redirection(args);

                if(redir_pos > 0)
                {
                    args[redir_pos] = NULL;
                    char* filename = args[redir_pos+1];
                    FILE *fptr = freopen(filename,"w", stdout);
                }

                int ss = execv(dest, args);

                if (ss == -1)
                {
                    print_error();
                }

                return 1;
            }
            else if (pid < 0)
            {
                print_error();
            }
            else
            {
                wait(NULL);
            }

            return 1;
        }
    }

    print_error();

    return 1;
}

int redirection(char** args)
{
    int args_len = args_length(args);
    int count_redirs = 0;
    int pos = 0;
    int file_count = 0;

    for (int i = 0; i < args_len; i++)
    {
        if (strcmp(args[i], ">") == 0)
        {
            count_redirs++;
            pos = i;
        }
    }

    int i = 1;

    while(args[pos+i] != NULL)
    {
        i++;
        file_count++;
    }

    if (file_count == 1 && count_redirs == 1)
    {
        return pos;
    }
    else if (file_count > 1 || count_redirs > 1)
    {
        print_error();
        exit(1);
    }

    return 0;
}

int args_to_path(int args_length, char** args)
{
    char** new_paths = malloc(20 * sizeof(char*));

    for (int i = 1; i < args_length; i++)
    {
        new_paths[i-1] = args[i];
    }

    PATH = new_paths;

    printf("NEW PATH :");
    for (int i = 0; i < args_length-1; i++)
    {
        printf("%s ", PATH[i]);
    }
    printf("\n");
    return 1;

}

int print_error()
{
    char error_message[30] = "An error has occurred\n";
    fprintf(stderr, error_message, strlen(error_message));
    return 0;
}
