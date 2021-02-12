#include <stdio.h>    // puts(), printf(), perror(), getchar()
#include <stdlib.h>   // exit(), EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h>   // getpid(), getppid(),fork()
#include <sys/wait.h> // wait()

#define STDIN  0
#define STDOUT 1

volatile sig_atomic_t SIGNAL = 0;

void child_a(int pfd[]) {
  close(pfd[0]);        // stänga vår read på pipe
  dup2(pfd[1], STDOUT); // ersätta stdout med vår pipebörjan
  close(pfd[1]);        // stänga vår pipebörjan
  int result = execlp("ls", "ls", "-F", "-1", NULL); // ls använder fd 1 som vi nu har ersatt med vår pipebörjan
  if (result < 0){
    perror("Creating pipe");
    exit(EXIT_FAILURE);
  }
}

void child_b(int pfd[]) {
  close(pfd[1]);      // stäng vår pipeöppning
  dup2(pfd[0], STDIN);// ersätt stdin med pipeslut
  close(pfd[0]);      // stänf pipeslut
  int result = execlp("nl", "nl", NULL);  // nl använder fd 0 där vi nu ersatt stdin med vårt pipeslut
  if (result < 0){
    perror("Creating pipe");
    exit(EXIT_FAILURE);
  }
}

int main(void) {
  int pfd[2];
  pid_t pid1;
  pid_t pid2;

  if (pipe(pfd) == -1) {
    perror("Creating pipe");
    exit(EXIT_FAILURE);
  }

  switch (pid1 = fork()) { //child a
  case -1:
    perror("fork failed");
    exit(EXIT_FAILURE);
  case 0:
    child_a(pfd);
  default:
    break;
  }

  switch (pid2 = fork()) { //child b
  case -1:
    perror("fork failed");
    exit(EXIT_FAILURE);
  case 0:
    child_b(pfd);
  default:
    break;
  }

  close(pfd[0]);
  close(pfd[1]);
  
  wait(&pid1);
  wait(&pid2);

  exit(EXIT_SUCCESS);
}
