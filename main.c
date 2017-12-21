//The shell's core is inspired by the work of S.Brennan.

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>


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
        &rpi_help,
        &rpi_clear,
        &rpi_exit,
        &rpi_hello,
        &rpi_raspInfo,
        &rpi_pigpiod_status
};

// Function "string call"
char *builtin_str[] = {
        "help",
        "cls",
        "exit",
        "hello",
        "rasp",
        "pigpio"
};

int rpi_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}


//Functions implementation.
int rpi_raspInfo(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "rasp command waits for argument, type -help for help");
  }

  else {
    // Option
    if (!strncmp(args[1], "-t",2)){
      // Get processor temperature
      system("cat /sys/class/thermal/thermal_zone0/temp > output.txt");
      outputFile = fopen("output.txt", "r+");
      char string[10];
      fgets(string, 10, outputFile);
      char *ptr;
      double ret;
      ret = strtod(string, &ptr);
      ret = ret / 1000;

      // Parameters
      if (args[2] == NULL || !strncmp(args[2], "cel",3)) {
        printf(" Processor temperature : %.2lf °C\n\n", ret);
      }

      if (!strncmp(args[2], "far",3)) {
        double retf = ret * 1.8 + 32;
        printf(" Processor temperature : %.2lf °F\n\n", retf);
      }

      if (!strncmp(args[2], "kel",3)) {
        double retk = ret + 273.15;
        printf(" Processor temperature : %.2lf °K\n\n", retk);
      }
    }

    if (!strncmp(args[1], "-ram",4)) {
      system("free -m | awk 'NR==2{printf \" RAM load : %.2f%%\\n\\n\", $3*100/$2 }'");

    }

    if (!strncmp(args[1], "-cpu",4)){
      system("top -bn1 | awk '/Cpu/ { cpu = \" CPU load :  \" 100 - $8 \"%\\n\" }; END   { print cpu }'");

    }

    if (!strncmp(args[1], "-h",2)){
      char str[128];
      strcat(str, " rasp -t [PARAM] | temp processor [PARAM] = Unity of temperature cel / far /kel\n");
      strcat(str, " rasp -cpu       | CPU usage\n");
      strcat(str, " rasp -ram       | RAM usage\n");

      printf"%s",str);
    }
  }

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

#define KWHI  "\x1B[97m"
#define RESET "\x1B[0m"
int rpi_help(char **args)
{

    char c;
    int i;    
    char str[1024];
    strcpy(str, " RPI SHELL Help\n");
    printf("%s",str);
    //Create a list to send the option -h into each functions.
    char *t[] = {NULL,"-h"};
    for (i = 4; i < rpi_num_builtins(); i++) {
      //Display the functions name 
      printf(KWHI"\n %s \n"RESET,builtin_str[i]);
      //execute the function with args[1] = -h
      (*builtin_func[i])(t);
    }
    strcpy(str,"\n by E. Albert, A. Hagopian and T. Selleslagh");
    strcat(str,"\n inspired by the work of S.Brennan.\n");
    printf("%s",str);
  return 1;
}

// returns the state of each gpio pin
int rpi_pigpiod_status(char **args)
{
    if (args[1] != NULL) {
        // help for the user
        if (! strncmp(args[1], "-h",2))
        {
            printf(" pigpio allows you to see the state of each pin\n");
            return 1;
        }
    }

    char str[100];

    for(int j=0; j < 26; j++)
    {

        int x = j;

        // slightly oversize buffer
        char buf[(sizeof(x) * CHAR_BIT) / 3 + 2];

        // index of next output digit
        char *result  = buf + sizeof(buf) - 1;

        // add digits to result, starting at
        // the end (least significant digit)

        *result = '\0'; // terminating null
        do {
            // remainder gives the next digit
            *--result = '0' + (x % 10);
            x /= 10;
            // keep going until x reaches zero
            } while (x);

        strcpy(str, " gpio");
        // add the integer to the string
        strcat(str, result);
        // make the displayed string nicer
        if(j < 10)
        {
            strcat(str, "  : ");
        }
        else{
            strcat(str, " : ");
        }
        // print the gpio pin number before its state
        printf("%s", str);
        fflush(stdout);
        strcpy(str, "echo ");
        strcat(str, result);
        strcat(str, " > /sys/class/gpio/export");
        // check if path exists
        if( access( str, F_OK ) != -1 ) {
            // file doesn't exist
            // so we create the gpio pin
            system(str);
        }
        strcpy(str, "cat ");
        strcat(str, "/sys/class/gpio/gpio");
        strcat(str, result);
        strcat(str, "/value");
        // reads the pin state
        system(str);

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
    _exit(EXIT_FAILURE);
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
    _exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();
    if (c == EOF) {
      _exit(EXIT_SUCCESS);
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
        _exit(EXIT_FAILURE);
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
    _exit(EXIT_FAILURE);
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
        _exit(EXIT_FAILURE);
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
