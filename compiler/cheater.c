#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdint.h>

#include <glib.h>
#include <glib/gprintf.h>

#include "types.h"
#include "cheater.h"

#define RD_END 0
#define WR_END 1

extern char** environ;

int redirectFd;
int oldStdout;

void cheaterInit() {
  umask(0);
  redirectFd = open(CHEATER_DUMPFILE, O_RDWR | O_CREAT | O_TRUNC, 0xffffffff);
  if ( redirectFd < 0 ) {
    fprintf(stderr, "Error: couldn't open dumpfile");
  }

  // save the old stdout and then set the redirected output for now
  oldStdout = dup(1);
  dup2(redirectFd, 1);
}

void runCheater(TreeNode* mainNode) {
  // return stdout
  fflush(stdout);
  dup2(oldStdout, 1);
  close(redirectFd);

  // create the pipe
  int fdesc[2]; // read: fdesc[0]
                // write: fdesc[1]
  int res = pipe(fdesc);
  if ( res < 0 ) {
    fprintf(stderr, "Error: failed to create pipe\n");
    exit(-1);
  }

  pid_t pid = fork();
  // if pid is 0, in child,
  // if nonzero, in parent...
  if ( pid == 0 ) {
    // replace stdout with fd
    close(fdesc[RD_END]);
    dup2(fdesc[WR_END], 1);

    // exec the new process
    execl("/bin/bash", "bash", "cheater-process.sh", NULL);
    perror("problem");
  }
  else {
    // close the write end
    close(fdesc[WR_END]);

    // parent will wait for the child and get status
    int status;
    waitpid(pid, &status, 0);

    if ( status != 0 ) {
      fprintf(stderr, "Error: Running program!\n");
      exit(-1);
    }

    // read data from the pipe
    GList* vals = NULL;
    int done = 0;
    while(!done) {
      char dataIn[500];
      int len = 0;
      done = 1;
      while ( read(fdesc[RD_END], &dataIn[len], 1) ) {
        if ( dataIn[len] == '\n' ) {
          done = 0;
          break;
        }
        len++;
      }
      dataIn[len] = 0;

      uint64_t outVal = atoi(dataIn);
      vals = g_list_append(vals, (int*)outVal);
    }

    GList* iter = vals;
    int instCount = 0;
    while ( iter != NULL && iter->next != NULL ) {
      printf("%i: LDC 0,%li(0)\n", instCount++, (uint64_t)vals->data);
      printf("%i: OUT 0,0,0\n", instCount++);
      iter = iter->next;
    }
  }
}
