#include "parser.h"    // cmd_t, position_t, parse_commands()

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>     //fcntl(), F_GETFL

#define STDIN  0
#define STDOUT 1

/**
 * For simplicitiy we use a global array to store data of each command in a
 * command pipeline .
 */
cmd_t commands[MAX_COMMANDS];

/**
 * For simplicitiy we use a global array to store pid of each command in a
 * command pipeline .
 */
pid_t pids[MAX_COMMANDS];

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
void fork_cmd(int i, int n, int pfd[]) {
  pid_t pid;
  position_t pos;
  switch (pid = fork()) {
    case -1:
      fork_error();
    case 0:
      // Child process after a successful fork().
      pos = cmd_position(i, n);
      if (pos == single){
        close(pfd[0]);
        close(pfd[1]);
      }
      else if (pos == first){
        puts("pos first");
        close(pfd[0]);        // stänga vår read på pipe
        dup2(pfd[1], STDOUT); // ersätta stdout med vår pipebörjan
        close(pfd[1]);        // stänga vår pipebörjan
      }
      else if (pos == middle){
        puts("pos middle/single");
        dup2(pfd[0], STDIN);
        dup2(pfd[1], STDOUT); // ersätta stdout med vår pipebörjan
        close(pfd[0]);
        close(pfd[1]); 
      }
      else if (pos == last){
        puts("pos last");
        close(pfd[1]);        // 
        dup2(pfd[0], STDIN); 
        close(pfd[0]);        
      }
      else if (pos == unknown){
        return;
      }
      // Execute the command in the contex of the child process.
      execvp(commands[i].argv[0], commands[i].argv);

      // If execvp() succeeds, this code should never be reached.
      fprintf(stderr, "shell: command not found: %s\n", commands[i].argv[0]);
      exit(EXIT_FAILURE);

    default:
      // Parent process after a successful fork().
      pids[i] = pid;
      break;
  }
}

/**
 *  Fork one child process for each command in the command pipeline.
 */
void fork_commands(int n, int pfd[]) {

  for (int i = 0; i < n; i++) {
    fork_cmd(i, n, pfd);
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
  pid_t pid;
  for (int i = 0; i < n; i++) {
    pid = pids[i];
    wait(&pid); // NOT DONE kan vara fel
  }
}

/**
 * Close remaining file descriptors.
 */
//void close_all(){}

int main() {
  int pfd[2];
  int n;               // Number of commands in a command pipeline.
  size_t size = 128;   // Max size of a command line string.
  char line[size];     // Buffer for a command line string.


  while(true) {
    printf(" >>> ");

    get_line(line, size);

    n = parse_commands(line, commands);
    fork_commands(n, pfd);

 //   close_all(); //TODO
    close(pfd[0]);
    close(pfd[1]);
    wait_for_all_cmds(n);
    print_commands(n);
  }

  exit(EXIT_SUCCESS);
}
