
/*
RISHA A
25040A_011
26/07/2026
PROJECT 1 - MINISHELL

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <signal.h>
#include "command.h"
#include <sys/types.h>

pid_t pid = 0;
int last_status = 0;
JOB *head = NULL;

int main()
{
    //clear the terminal
    system("clear");

    extract_external_commands(external_commands);

    char prompt[100] = "minishell$";
  
    char shell_path[PATH_MAX];

    ssize_t len = readlink("/proc/self/exe", shell_path, sizeof(shell_path) - 1);

    if (len != -1)
    {
      shell_path[len] = '\0';
    }
    else
    {
       strcpy(shell_path, "Unknown");
    }

//registering the signal
    signal(SIGINT, signal_handler);
    signal(SIGTSTP, signal_handler);
    signal(SIGCHLD, signal_handler);
    while (1)
    {
        char str[100];
        printf("%s ", prompt);
        fflush(stdout);

        if (fgets(str, sizeof(str), stdin) == NULL)
        {
            continue;
        }

        str[strcspn(str, "\n")] = '\0';
//pipe check
        if (strchr(str, '|') != NULL)
        {
            execute_pipe(str);

            continue;
        }
//check PS1 preent or not
        if (strncmp(str, "PS1=", 4) == 0)
        {
            if (strchr(str, ' ') == NULL)
            {
                strcpy(prompt, str + 4);

                continue;
            }
        }

        char *args[100];
        int i = 0;

        args[i] = strtok(str, " ");

        while (args[i] != NULL)
        {
            i++;

            args[i] = strtok(NULL, " ");
        }
        int bg = 0;

        if (i > 0 && strcmp(args[i - 1], "&") == 0)
        {
            bg = 1;

            args[i - 1] = NULL;
        }

        if (args[0] == NULL)
        {
            continue;
        }

        if (check_builtin(args[0]))
        {
            execute_internal(args, shell_path);
        }

        else if (check_external(args[0]))
        {
            pid = fork();

            if (pid > 0)
            {
                if (bg == 0)
                {
                    int status;

                    waitpid(pid, &status, WUNTRACED);

                    if (WIFEXITED(status))
                    {
                        last_status = WEXITSTATUS(status);
                    }

                    pid = 0;
                }

                else
                {
                    printf("[%d]\n", pid);

                    insert_at_first(pid, args[0]);

                    pid = 0;
                }
            }

            else if (pid == 0)
            {
                signal(SIGINT, SIG_DFL);

                signal(SIGTSTP, SIG_DFL);

                execute_external(args);
            }

            else
            {
                perror("fork");
            }
        }

        else
        {
            printf("Command not found\n");
        }
    }

    return 0;
}
