#include "parser.h"    // cmd_t, position_t, parse_commands()

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>     //fcntl(), F_GETFL

#define STD_IN  0
#define STD_OUT 1

/**
 * For simplicitiy we use a global array to store data of each command in a
 * command pipeline .
 */
cmd_t commands[MAX_COMMANDS];

/**
 *  Debug printout of the commands array.
 */
void print_commands(int n) {
  for (int i = 0; i < n; i++) {
    printf("==> commands[%d]\n", i);
    printf("  pos = %s\n", position_to_string(commands[i].pos));
    printf("  in  = %d\n", commands[i].in);
    printf("  out = %d\n", commands[i].out);

    print_argv(commands[i].argv);
  }

}

/**
 * Returns true if file descriptor fd is open. Otherwise returns false.
 */
int is_open(int fd) {
  return fcntl(fd, F_GETFL) != -1 || errno != EBADF;
}

void fork_error() {
  perror("fork() failed)");
  exit(EXIT_FAILURE);
}

/**
 *  Fork a proccess for command with index i in the command pipeline. If needed,
 *  create a new pipe and update the in and out members for the command..
 */
void fork_cmd(int i, int n) {
  pid_t pid;

  switch (pid = fork()) {
    case -1:
      fork_error();
    case 0:
      // Child process after a successful fork().
      position_t pos = cmd_position(i, n);
      if (pos == single){
        
      }
      if (pos == first){
        close(pfd[0]);        // stänga vår read på pipe
        dup2(pfd[1], STDOUT); // ersätta stdout med vår pipebörjan
        close(pfd[1]);        // stänga vår pipebörjan
      }
      if (pos == middle){
        return;
      }
      if (pos == last){
        close(pfd[1]);        // stänga vår read på pipe
        dup2(pfd[0], STDOUT); // ersätta stdout med vår pipebörjan
        close(pfd[0]);        // stänga vår pipebörjan
      }
      if (pos == unknown){
        return;
      }
      // Execute the command in the contex of the child process.
      execvp(commands[i].argv[0], commands[i].argv);

      // If execvp() succeeds, this code should never be reached.
      fprintf(stderr, "shell: command not found: %s\n", commands[i].argv[0]);
      exit(EXIT_FAILURE);

    default:
      // Parent process after a successful fork().

      break;
  }
}

/**
 *  Fork one child process for each command in the command pipeline.
 */
void fork_commands(int n) {

  for (int i = 0; i < n; i++) {
    fork_cmd(i, n);
  }
}

/**
 *  Reads a command line from the user and stores the string in the provided
 *  buffer.
 */
void get_line(char* buffer, size_t size) {
  getline(&buffer, &size, stdin);
  buffer[strlen(buffer)-1] = '\0';
}

/**
 * Make the parents wait for all the child processes.
 */
void wait_for_all_cmds(int n) {
  for (int i = 0; i < n; i++) {
    wait(i); // NOT DONE kan vara fel
  }
}

/**
 * Close remaining file descriptors.
 */
void close_all(){
  return;
}

int main() {
  int pfd[2];
  int n;               // Number of commands in a command pipeline.
  size_t size = 128;   // Max size of a command line string.
  char line[size];     // Buffer for a command line string.


  while(true) {
    printf(" >>> ");

    get_line(line, size);

    n = parse_commands(line, commands);

    fork_commands(n);

    close_all(); //TODO

    wait_for_all_cmds(n);
  }

  exit(EXIT_SUCCESS);
}
