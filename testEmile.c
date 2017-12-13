/***************************************************************************//**
  @file         main.c
  @author       Stephen Brennan
  @date         Thursday,  8 January 2015
  @brief
*******************************************************************************/

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Function Declarations for builtin shell commands :
int rpi_exit(char **args);
int rpi_hello(char **args);
int rpi_raspInfo(char **args);

FILE* outputFile;


// List of builtin commands, followed by their corresponding functions
char *builtin_str[] = {
        "exit",
        "hello",
        "rasp"
};

int (*builtin_func[]) (char **) = {
        &rpi_exit,
        &rpi_hello,
        &rpi_raspInfo,
};

int rpi_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

// Builtin function implementations
/**
   @brief Bultin command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
*/

int rpi_raspInfo(char **args)
{
    // /proc/self/status
    // Utilisation de la ram - free -h
    // CPU htop

    // Processor temperature
    system("cat /sys/class/thermal/thermal_zone0/temp > output.txt");
    outputFile = fopen("output.txt", "r+");
    char string[10];
    fgets(string, 10, outputFile);
    char *ptr;
    double ret;

    ret = strtod(string, &ptr);
    ret = ret/1000;
    printf("Processor temperature : %.2lf\n", ret);
    return 1;
}

int rpi_hello(char **args)
{
    printf("Hello World ! \n");
    return 1;
}

/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int rpi_exit(char **args)
{
    return 0;
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */

int rpi_launch(char **args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("rpi");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("rpi");
    } else {
        // Parent process
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int rpi_execute(char **args)
{
    int i;

    if (args[0] == NULL) {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < rpi_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return rpi_launch(args);
}

#define rpi_RL_BUFSIZE 1024
/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *rpi_read_line(void)
{
    int bufsize = rpi_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "rpi: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read a character
        c = getchar();

        if (c == EOF) {
            exit(EXIT_SUCCESS);
        } else if (c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        // If we have exceeded the buffer, reallocate.
        if (position >= bufsize) {
            bufsize += rpi_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "rpi: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define rpi_TOK_BUFSIZE 64
#define rpi_TOK_DELIM " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **rpi_split_line(char *line)
{
    int bufsize = rpi_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token, **tokens_backup;

    if (!tokens) {
        fprintf(stderr, "rpi: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, rpi_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += rpi_TOK_BUFSIZE;
            tokens_backup = tokens;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                free(tokens_backup);
                fprintf(stderr, "rpi: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, rpi_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void rpi_loop(void)
{
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = rpi_read_line();
        args = rpi_split_line(line);
        status = rpi_execute(args);

        free(line);
        free(args);
    } while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{
    // Load config files, if any.

    // Run command loop.
    system("touch /home/pi/output.txt");
    rpi_loop();

    // Perform any shutdown/cleanup.

    return EXIT_SUCCESS;
}

