#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define ASH_RL_BUFF_SIZE 1024
#define ASH_TOK_BUFF_SIZE 64
#define ASH_TOK_DELIM "\t\r\n\a "

int ash_cd(char **args);
int ash_help(char **args);
int ash_exit(char **args);

char *builtin_str[] = {"cd", "help", "exit"};
int (*builtin_func[])(char **) = {&ash_cd, &ash_help, &ash_exit};
int ash_num_builtins() { return sizeof(builtin_str) / sizeof(char *); }

int ash_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "ash: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("ash");
    }
  }
  return 1;
}

int ash_help(char **args) {
  int i;
  printf("Adam Azuddin's ASH\n");
  printf("Type programme name and arguments, and hit enter\n");
  printf("The followings are built in:\n");

  for (i = 0; i < ash_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs\n");
  return 1;
}

int ash_exit(char **args) { return 0; }

char *ash_read_line(void) {
  int buffSize = ASH_RL_BUFF_SIZE;
  int pos = 0;
  char *buffer = malloc(sizeof(char) * buffSize);
  int c;
  char *line;

  if (!buffer) {
    fprintf(stderr, "ash: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = getchar();
    if (c == EOF || c == '\n') {
      buffer[pos++] = '\0';
      return buffer;
    } else {
      buffer[pos++] = c;
    }

    if (pos >= buffSize) {
      buffSize += ASH_RL_BUFF_SIZE;
      buffer = realloc(buffer, buffSize);
      if (!buffer) {
        fprintf(stderr, "ash: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
  return line;
}

char **ash_split_line(char *line) {
  int buffSize = ASH_TOK_BUFF_SIZE;
  int pos = 0;
  char **tokens = malloc(sizeof(char) * buffSize);
  char *token;

  if (!tokens) {
    fprintf(stderr, "ash: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, ASH_TOK_DELIM);

  while (token != NULL) {
    tokens[pos] = token;
    pos++;

    if (pos >= buffSize) {
      buffSize += ASH_TOK_BUFF_SIZE;
      tokens = realloc(tokens, sizeof(char) * buffSize);

      if (!tokens) {
        fprintf(stderr, "ash: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, ASH_TOK_DELIM);
  }
  tokens[pos] = NULL;
  return tokens;
}

int ash_launch(char **args) {
  pid_t pid;
  int status;

  pid = fork();

  if (pid == 0) {
    // child process
    if (execvp(args[0], args) == -1) {
      perror("ash");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("ash");
  } else {
    // parent process
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int ash_execute(char **args) {
  int i;
  if (args[0] == NULL) {
    return 1;
  }

  for (i = 0; i < ash_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return ash_launch(args);
}
void ash_loop() {
  char *line;
  char **args;
  int status;

  do {
    printf("ash\t>  ");
    line = ash_read_line();
    args = ash_split_line(line);
    status = ash_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char *argv[]) {
  ash_loop();
  return EXIT_SUCCESS;
}
