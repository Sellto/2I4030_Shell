//The shell's core is inspired by the work of S.Brennan.

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



// Function Declarations
int rpi_exit(char **args);
int rpi_hello(char **args);
int rpi_raspInfo(char **args);
int rpi_clear(char **args);
int rpi_help(char **args);
int rpi_pigpiod_status(char **args);

FILE* outputFile;

//Pointer to the functions
int (*builtin_func[]) (char **) = {
        &rpi_exit,
        &rpi_hello,
        &rpi_raspInfo,
        &rpi_clear,
        &rpi_help,
        &rpi_pigpiod_status
};

// Function "string call"
char *builtin_str[] = {
        "exit",
        "hello",
        "rasp",
        "cls",
        "help",
        "pigpio"
};

int rpi_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}


//Functions implementation.
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

int rpi_clear(char **args)
{
    system("clear");
    return 1;
}

int rpi_help(char **args)
{
    printf("Help\n");
    return 1;
}

int rpi_pigpiod_status(char **args)
{
    if (args[1] == NULL) {
        printf("noargument\n");
    } else {
       if (! strncmp(args[1], "-s",2))
       {
           printf("valid\n");
       }
       else
       {
           fprintf(stderr, "Invalid Argument\n");
       }
    }
    return 1;
}

int rpi_exit(char **args)
{
  return 0;
}

//Shell Core
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

//Pick the user entry.
#define rpi_RL_BUFSIZE 1024
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


//"Decode" the user entry.
#define rpi_TOK_BUFSIZE 64
#define rpi_TOK_DELIM " \t\r\n\a"
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

//SuperLoop
#define KBLU  "\x1B[32m"
#define RESET "\x1B[0m"
void rpi_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf(KBLU" RPi-:> "RESET);
    line = rpi_read_line();
    args = rpi_split_line(line);
    status = rpi_execute(args);
    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv)
{
  system("touch /home/pi/output.txt");
  system("clear");
  rpi_loop();
  system("clear");
  return EXIT_SUCCESS;
}