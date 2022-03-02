#include <signal.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "defs.h"

extern int LINES, COLS;

int process(char **cmd)
{
    int retstat, pid, waitstat;

    if ( (pid = fork()) == 0) {
      resetty();
      execvp(*cmd, cmd);
      exit(0);
    }
    setty();
    /* Check to see if fork failed. */
    if (pid < 0) 
      qx_exit(1);
    while ( (waitstat = wait(&retstat)) != pid && waitstat != -1 ) 
      ;
    setty();
    if (waitstat == -1) 
      retstat = -1;

    return(retstat);
}


void exec_shell()
{
  char *arr[2];

  arr[0] = (char *)getenv("SHELL");
  arr[1] = (char *)0;

  gotoxy(0,LINES-1);
  clrtoeol();
  process(arr);
}
