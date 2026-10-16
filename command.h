#ifndef COMMAND_H
#define COMMAND_H

#define MAX_SIZE 1024

// structure for job
typedef struct node
{
    int pid;
    char cmd[100];
    struct node *link;

} JOB;

extern pid_t pid;
extern int last_status;
extern JOB *head;

extern char *buildin[];
extern char *external_commands[500];
void extract_external_commands(char **external_commands);int check_builtin(char *cmd);
int check_external(char *cmd);
void execute_external(char *args[]);
void execute_internal(char *args[], char shell_path[]);
void insert_at_first(int pid, char *cmd);
void delete_at_first();
void print_jobs();
void signal_handler(int signum);
void execute_pipe(char *str);
extern int last_status;

#endif