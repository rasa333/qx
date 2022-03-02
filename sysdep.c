#include "config.h"
#include <termio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>
#include "defs.h" 
#include <sys/time.h>
#include <sys/types.h>

struct termio otio,ntio;



int __setty_called = 0;
void catchit(int);


int setty()
{
  if (!__setty_called) {
    ioctl(0,TCGETA,&otio);   
    ntio = otio;
    ntio.c_lflag &= ~ECHO;
    ntio.c_lflag &= ~ICANON;
    ntio.c_oflag |= ONLCR;
    ntio.c_iflag |= ICRNL;
    ntio.c_cc[VMIN]  = 1;
    ntio.c_cc[VTIME] = 0;
    ntio.c_cc[VINTR] = 0;
    ioctl(0,TCSETAW,&ntio);
    __setty_called = 1;
  }
  ioctl(0,TCSETAW,&ntio);
  signal(SIGINT,SIG_IGN);
  signal(SIGSEGV,catchit);
  signal(SIGQUIT,catchit);
  signal(SIGTERM,catchit);
}


void resetty()
{
  if (__setty_called) {
    ioctl(0,TCSETAW,&otio);
  }
  signal(SIGINT,SIG_DFL);
  signal(SIGSEGV,SIG_DFL);
  signal(SIGQUIT,SIG_DFL);
  signal(SIGTERM,SIG_DFL);
  alarm(0);
  signal(SIGALRM,SIG_DFL);
}

int kbhit()
{
  struct timeval timeout;
  fd_set rd;

  FD_ZERO (&rd);
  FD_SET (0, &rd);
 
  timeout.tv_sec = 0;
  timeout.tv_usec = 50;

  /* wait for input */
  return select (1, &rd, (fd_set *)0, (fd_set *)0, &timeout);
}


void catchit(int sig)
{
  signal(sig, SIG_IGN);
  qx_exit(-1);
}




















