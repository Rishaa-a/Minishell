#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <signal.h>
#include <sys/types.h>
#include "command.h"

 
char *external_commands[500];

char *buildin[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
                   "set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
                   "exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help","fg","bg","jobs",NULL
                  };


void signal_handler(int signum)
{
    if (signum == SIGINT)
    {
        if (pid == 0)
        {
            printf("\nminishell$");

            fflush(stdout);
        }
        else
        {
            kill(pid, SIGINT);
        }
    }

    else if (signum == SIGTSTP)
    {
        if (pid == 0)
        {
            printf("\nminishell$");

            fflush(stdout);
        }

        else
        {
            kill(pid, SIGTSTP);

            printf("\n[%d]+ Stopped\n", pid);

            insert_at_first(pid, "Stopped Process");

            fflush(stdout);
        }
    }

    else if (signum == SIGCHLD)
    {
      int status;
      pid_t child_pid;

      while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0)
      {
        if (WIFEXITED(status))
        {
            printf("\n[%d] Done\tExit Status : %d\n",
                   child_pid, WEXITSTATUS(status));
        }

        fflush(stdout);
      }
    }
}

void print_jobs()
{
    JOB *temp = head;

    while (temp != NULL)
    {
        printf("[%d] %s\n", temp->pid, temp->cmd);

        temp = temp->link;
    }
}

void delete_at_first()
{
    if (head == NULL)
    {
        return;
    }

    JOB *temp = head;

    head = head->link;

    free(temp);
}

void insert_at_first(int pid, char *cmd)
{
    JOB *new = malloc(sizeof(JOB));

    new->pid = pid;

    strcpy(new->cmd, cmd);

    new->link = head;

    head = new;
}

void execute_internal(char *args[], char shell_path[])
{
    if (strcmp(args[0], "cd") == 0)
    {
        chdir(args[1]);
    }

    else if (strcmp(args[0], "pwd") == 0)
    {
        char cwd[PATH_MAX];

        getcwd(cwd, sizeof(cwd));

        printf("%s\n", cwd);
    }

    else if (strcmp(args[0], "exit") == 0)
    {
        exit(0);
    }

    else if (strcmp(args[0], "echo") == 0)
    {
        if (args[1] != NULL)
        {

            if (strcmp(args[1], "$$") == 0)
            {
                printf("%d\n", getpid());
            }

            else if (strcmp(args[1], "$?") == 0)
            {
                printf("%d\n", last_status);
            }

            else if (strcmp(args[1], "$SHELL") == 0)
            {
                printf("%s\n", shell_path);
            }
        }
    }
    else if (strcmp(args[0], "jobs") == 0)
    {
        print_jobs();
    }
    else if (strcmp(args[0], "fg") == 0)
    {
        if (head != NULL)
        {
            pid = head->pid;

            kill(pid, SIGCONT);

            waitpid(pid, NULL, 0);

            delete_at_first();

            pid = 0;
        }
    }
    else if (strcmp(args[0], "bg") == 0)
    {
        if (head != NULL)
        {
            kill(head->pid, SIGCONT);

            delete_at_first();
        }
    }
}

void execute_pipe(char *str)
{
    char *commands[10];

    int n = 0;

    commands[n] = strtok(str, "|");

    while (commands[n] != NULL)
    {
        n++;

        commands[n] = strtok(NULL, "|");
    }

    int in = 0;

    int fd[2];

    for (int i = 0; i < n; i++)
    {
        pipe(fd);

        pid_t pid = fork();

        if (pid == 0)
        {
            dup2(in, 0);

            if (i != n - 1)
            {
                dup2(fd[1], 1);
            }

            close(fd[0]);

            char *args[20];

            int j = 0;

            args[j] = strtok(commands[i], " \n");

            while (args[j] != NULL)
            {
                j++;

                args[j] = strtok(NULL, " \n");
            }

            execvp(args[0], args);

            perror("execvp");

            exit(1);
        }

        else
        {
            wait(NULL);

            close(fd[1]);

            in = fd[0];
        }
    }
}

void extract_external_commands(char **external_commands)
{
    FILE *fp = fopen("cmd.txt", "r");

    if (fp == NULL)
    {
        perror("fopen");
        exit(1);
    }

    char buffer[153];
    int i = 0;

    while (fgets(buffer, sizeof(buffer), fp))
    {
        buffer[strcspn(buffer, "\r\n")] = '\0';

        external_commands[i] = malloc(strlen(buffer) + 1);

        if (external_commands[i] == NULL)
        {
            perror("malloc");
            exit(1);
        }

        strcpy(external_commands[i], buffer);
        i++;
    }

    external_commands[i] = NULL;

    fclose(fp);
}

void execute_external(char *args[])
{
    execvp(args[0], args);

    perror("execvp");

    exit(1);
}
int check_builtin(char *cmd)
{
    int i = 0;

    while (buildin[i] != NULL)
    {
        if (strcmp(cmd, buildin[i]) == 0)
        {
            return 1;
        }

        i++;
    }

    return 0;
}

int check_external(char *cmd)
{
    int i = 0;

    while (external_commands[i] != NULL)
    {
        if (strcmp(cmd, external_commands[i]) == 0)
        {
            return 1;
        }

        i++;
    }

    return 0;
}


