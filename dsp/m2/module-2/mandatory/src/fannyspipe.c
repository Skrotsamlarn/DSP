#include <stdio.h>    // puts(), printf(), perror(), getchar()
#include <stdlib.h>   // exit(), EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h>   // getpid(), getppid(),fork()
#include <sys/wait.h> // wait()

#define READ  0
#define WRITE 1


void child(int fd[]){
  close(fd[READ]);

  dup2(fd[0], WRITE);

  close(fd[READ]);

  // Execute wc
  execlp("wc", "wc", NULL);
}



void child_a(int fd[]) {
puts("barn1");
dup2(fd[0], WRITE);
execlp("ls", "ls", "-F", "-1", NULL);
}

void child_b(int fd[]) {
puts("barn1");
dup2(fd[1], READ);

execlp("which", "nl", NULL);
exit(2);
}

void parent(pid_t pid){
  wait(&pid);
}

int main(void) {
  int fd[2];
  pid_t pid;

  if (pipe(fd) == -1) {
    perror("Creating pipe");
    exit(EXIT_FAILURE);
  }

  switch (pid = fork()) { //child a
  case -1:
    perror("fork failed");
    exit(EXIT_FAILURE);
  case 0:
    child_a(&fd[0]);
  default:
    puts("pappan2");
    parent(pid);
  }

  switch (pid = fork()) { //child b
  case -1:
    perror("fork failed");
    exit(EXIT_FAILURE);
  case 0:
    child_b(&fd[1]);
  default:
  puts("pappan1");
    parent(pid);
  }

}
