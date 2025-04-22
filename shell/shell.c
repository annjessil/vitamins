#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "tokenizer.h"

/* Convenience macro to silence compiler warnings about unused function
 * parameters. */
#define unused __attribute__((unused))

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens *tokens);
int cmd_help(struct tokens *tokens);
int cmd_pwd(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
    cmd_fun_t *fun;
    char *cmd;
    char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
    {cmd_help, "?", "show this help menu"},
    {cmd_exit, "exit", "exit the command shell"},
    {cmd_pwd, "pwd", "prints the current working directory"}, 
    {cmd_cd, "cd", "changes the current working directory to new directory"}
};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens *tokens) {
    for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++) {
        printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
    }
    return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens *tokens) {
    exit(0);
}

int cmd_pwd(unused struct tokens *tokens){
    //int size = cmd_fun_t(&tokens);
    char cwd[1024]; //change 1024 later
   if (getcwd(cwd, sizeof(cwd)) != NULL) {
       printf("%s\n", cwd);
   } else {
       perror("getcwd() error");
       return 1;
   }
   return 0;
}

int cmd_cd(unused struct tokens *tokens){
    char *path = tokens_get_token(tokens, 1); //getting path
    if (path == NULL) {
        fprintf(stderr, "cd: missing argument\n");
        return 1;
    }

    chdir(path);
    return 0;
}

/* Looks up the built-in command, if it exists. */
int lookup(char *cmd) {
    if (cmd != NULL) {
        for (int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++) {
            if (strcmp(cmd_table[i].cmd, cmd) == 0) {
                return i;
            }
        }
    }
    return -1;
}

/* Intialization procedures for this shell */
void init_shell() {
    /* Our shell is connected to standard input. */
    shell_terminal = STDIN_FILENO;

    /* Check if we are running interactively */
    shell_is_interactive = isatty(shell_terminal);

    if (shell_is_interactive) {
        /* If the shell is not currently in the foreground, we must pause the
         * shell until it becomes a foreground process. We use SIGTTIN to pause
         * the shell. When the shell gets moved to the foreground, we'll receive
         * a SIGCONT. */
        while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp())) {
            kill(-shell_pgid, SIGTTIN);
        }

        /* Saves the shell's process id */
        shell_pgid = getpid();

        /* Take control of the terminal */
        tcsetpgrp(shell_terminal, shell_pgid);

        /* Save the current termios to a variable, so it can be restored later.
         */
        tcgetattr(shell_terminal, &shell_tmodes);
    }
}

int main(unused int argc, unused char *argv[]) {
    init_shell();

    static char line[4096];
    int line_num = 0;
    int pid;

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);
    sigaction(SIGTTOU, &sa, NULL);
    
    /* Only print shell prompts when standard input is not a tty */
    if (shell_is_interactive) {
        fprintf(stdout, "%d: ", line_num);
    }

    while (fgets(line, 4096, stdin)) {
        /* Split our line into words. */
        struct tokens *tokens = tokenize(line);

        /* Find which built-in function to run. */
        int fundex = lookup(tokens_get_token(tokens, 0));

        if (fundex >= 0) {
            cmd_table[fundex].fun(tokens);
        } else {
            /* REPLACE this to run commands as programs. */            
            const char *path = getenv("PATH"); //for no explicit path
            char *pathCopy = strdup(path);
            char *dir = strtok(pathCopy, ":");

            char *command = tokens_get_token(tokens, 0); //for explicit path
            int size = tokens_get_length(tokens);
            char *args[size + 1];

            int inputFD = -1, outputFD = -1;    //input & output redirection
            char *infile = NULL, *outfile = NULL;


            for (int i = 0; i < size; i++){
                args[i] = tokens_get_token(tokens, i);
            }

            args[size] = NULL;

            //signal(SIGINT, SIG_IGN);
            //signal(SIGTSTP, SIG_IGN);
            //signal(SIGTTOU, SIG_IGN);

            pid = fork(); 

            if (pid == 0) { // child process
                
                struct sigaction sa_default;
                sa_default.sa_handler = SIG_DFL;
                sigemptyset(&sa_default.sa_mask);
                sa_default.sa_flags = 0;
                sigaction(SIGINT, &sa_default, NULL);
                sigaction(SIGTSTP, &sa_default, NULL);
                sigaction(SIGTTOU, &sa_default, NULL);
                
                //tcsetpgrp(shell_terminal, pid);
                //signal(SIGINT, SIG_DFL);
                //signal(SIGTSTP, SIG_DFL);

                setpgid(0, 0); //since pid equals 0
                tcsetpgrp(shell_terminal, getpid()); // child is in foreground
               

                for (int i = 0; i < size; i++){
                    if ((strcmp(args[i], "<") == 0) && ((i+1) < size)){
                        //Similarly, the syntax ”[process] < [file]” tells your shell to feed the contents of a file to the process’s standard input
                        inputFD = i;
                        infile = args[i+1];
                        args[inputFD] = NULL;
                    } else if ((strcmp(args[i], ">") == 0) && ((i+1) < size)){
                        //The syntax “[process] > [file]” tells your shell to redirect the process’s standard output to a file. 
                        outputFD = i;
                        outfile = args[i + 1];
                        args[outputFD] = NULL;
                    }
                }

                if (infile != NULL) { //opening file for redirection
                    int fd = open(infile, O_RDONLY);
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }

                if (outfile != NULL){ //opening file for redirection
                    int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
                
                if (strchr(command, '/') != NULL) { //explicit path
                    execv(command, args);
                    exit(0);
                } else {
                    while (dir != NULL){ //no explicit path
                        char pathVar[1024];
                        snprintf(pathVar, sizeof(pathVar), "%s/%s", dir, command); //$PATH + command

                        if (access(pathVar, X_OK) == 0) { //finding correct path
                            execv(pathVar, args);
                            exit(0);
                        }
                        dir = strtok(NULL, ":");
                    }

                    exit(1); //catch
                } 
                             
            } else { // parent process
                int status;
                signal(SIGTTOU, SIG_IGN);
                tcsetpgrp(shell_terminal, pid);
                signal(SIGTTOU, SIG_DFL);

                waitpid(pid, &status, 0);  //waiting for children to finish
                tcsetpgrp(shell_terminal, shell_pgid); // take terminal back
            }

            free(pathCopy); //freeing memory from strdup
        
        }
        

        if (shell_is_interactive) {
            /* Only print shell prompts when standard input is not a tty. */
            fprintf(stdout, "%d: ", ++line_num);
        }

        /* Clean up memory. */
        tokens_destroy(tokens);
    }
    return 0;
}
