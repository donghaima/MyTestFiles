#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


void
waitdaemon_timeout (int signo)
{
  int left;

  (void)signo;
  left = alarm (0);
  signal (SIGALRM, SIG_DFL);
  if (left == 0) {
      fprintf(stderr, "timed out waiting for child");
      exit(1);
  }
}


/* waitdaemon is like daemon, but optionally the parent pause up
   until maxwait before exiting. Return -1, on error, otherwise
   waitdaemon will return the pid of the parent.  */

int
waitdaemon (int nochdir, int noclose, int maxwait)
{
  int fd;
  pid_t childpid;
  pid_t ppid;

  ppid = getpid ();

  switch (childpid = fork ())
    {
    case -1: /* Something went wrong.  */
      return (-1);

    case 0:  /* In the child.  */
      break;

    default:   /* In the parent.  */
      if (maxwait > 0)
	{
	  signal (SIGALRM, waitdaemon_timeout);
	  alarm (maxwait);
	  pause ();
	}
      _exit(0);
    }

  if (setsid () == -1)
    return -1;

  /* SIGHUP is ignore because when the session leader terminates
     all process in the session (the second child) are sent the SIGHUP.  */
  signal (SIGHUP, SIG_IGN);

  switch (fork ())
    {
    case 0:
      break;

    case -1:
      return -1;

    default:
      _exit (0);
    }

  if (!nochdir)
    chdir ("/");

  if (!noclose)
    {
      int i;
      long fdlimit = -1;

      fdlimit = sysconf(_SC_OPEN_MAX);

      if (fdlimit == -1)
          fdlimit = 64;

      for (i = 0; i < fdlimit; i++)
	close (i);

      fd = open ("/dev/null", O_RDWR, 0);
      if (fd != -1)
	{
	  dup2 (fd, STDIN_FILENO);
	  dup2 (fd, STDOUT_FILENO);
	  dup2 (fd, STDERR_FILENO);
	  if (fd > 2)
	    close (fd);
	}
    }
  return ppid;
}




int main(void) 
{
    pid_t ppid = 0;

    ppid = waitdaemon (0, 0, 1);
    if (ppid < 0)
    {
        fprintf (stderr, "could not become daemon: %s\n",
                 strerror(errno));
        return (-1);
    }

    return 0;
}
