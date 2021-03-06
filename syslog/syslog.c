/*
 * Copyright (c) 1983, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)syslog.c	8.4 (Berkeley) 3/18/94";
#endif /* LIBC_SCCS and not lint */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define _GNU_SOURCE  /* for open_memstream() */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <sys/uio.h>
#include <netdb.h>

#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define _LIBC 2
#define  NOT_IN_libc
#if _LIBC - 0
#include <bits/libc-lock.h>
#endif
#undef _LIBC


#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#define _LIBC 2
#define __GNUC__ 3


static int	LogType = SOCK_DGRAM;	/* type of socket connection */
static int	LogFile = -1;		/* fd for log */
static int	connected;		/* have done connect */
static int	LogStat = 0;		/* status bits, set by openlog() */
static const char *LogTag = NULL;	/* string to tag the entry with */
static int	LogFacility = LOG_USER;	/* default facility code */
static int	LogMask = 0xff;		/* mask of priorities to be logged */
extern char	*__progname;		/* Program name, from crt0. */

#if _LIBC - 0
/* Define the lock.  */
__libc_lock_define_initialized (static, syslog_lock)
#endif

static void openlog_internal(const char *, int, int);
static void closelog_internal(void);
static void sigpipe_handler (int);
#if _LIBC - 0 && defined (_LIBC_REENTRANT)
static void cancel_handler (void *);
#endif

/*
 * syslog, vsyslog --
 *	print message on log file; output is intended for syslogd(8).
 */

void
#if __STDC__
syslog(int pri, const char *fmt, ...)
#else
syslog(pri, fmt, va_alist)
	int pri;
	char *fmt;
	va_dcl
#endif
{
	va_list ap;

#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	vsyslog(pri, fmt, ap);
	va_end(ap);
}

void
vsyslog(pri, fmt, ap)
	int pri;
	register const char *fmt;
	va_list ap;
{
	struct tm now_tm;
	time_t now;
	int fd;
	FILE *f;
	char *buf = 0;
	size_t bufsize = 0;
	size_t prioff, msgoff;
 	struct sigaction action, oldaction;
	struct sigaction *oldaction_ptr = NULL;
#if !_LIBC
	char tbuf[2048], fmt_cpy[1024];
#endif
 	int sigpipe;

#define	INTERNALLOG	LOG_ERR|LOG_CONS|LOG_PERROR|LOG_PID
	/* Check for invalid bits. */
	if (pri & ~(LOG_PRIMASK|LOG_FACMASK)) {
		syslog(INTERNALLOG,
		    "syslog: unknown facility/priority: %x", pri);
		pri &= LOG_PRIMASK|LOG_FACMASK;
	}

	/* Check priority against setlogmask values. */
	//if (!LOG_MASK(LOG_PRI(pri)) & LogMask) {   // This is WRONG!!
        if ((LOG_MASK (LOG_PRI (pri)) & LogMask) == 0) {
            printf("Failed logmask check, return! "
                   "LOG_MASK(LOG_PRI(%d))=%d, LogMask=%d)\n",
                   pri, LOG_MASK(LOG_PRI(pri)), LogMask);

            return;
        } else 
            printf("logmask check ok: LOG_MASK(LOG_PRI(%d))=%d, LogMask=%d)\n",
                   pri, LOG_MASK(LOG_PRI(pri)), LogMask);

	/* Set default facility if none specified. */
	if ((pri & LOG_FACMASK) == 0)
		pri |= LogFacility;

#if _LIBC - 0
	/* Build the message in a memory-buffer stream.  */

	f = open_memstream (&buf, &bufsize);

	prioff = fprintf (f, "<%d>", pri);
	(void) time (&now);
#ifdef USE_IN_LIBIO
        f->_IO_write_ptr += strftime (f->_IO_write_ptr,
                                      f->_IO_write_end - f->_IO_write_ptr,
                                      "%h %e %T ",
				      __localtime_r (&now, &now_tm));
#else
	//f->__bufp += strftime (f->__bufp, f->__put_limit - f->__bufp,
                               //"%h %e %T ", __localtime_r (&now, &now_tm));
        char datestring[256];
        strftime (datestring, sizeof(datestring),
                  "%h %e %T", localtime_r(&now, &now_tm));
        fprintf (f, "%s ", datestring);
#endif
	msgoff = ftell (f);
	if (LogTag == NULL)
	  LogTag = __progname;
	if (LogTag != NULL)
	  fputs (LogTag, f);
	if (LogStat & LOG_PID)
	  fprintf (f, "[%d]", getpid ());
	if (LogTag != NULL)
	  putc (':', f), putc (' ', f);

	/* We have the header.  Print the user's format into the buffer.  */
	vfprintf (f, fmt, ap);

	/* Close the memory stream; this will finalize the data
	   into a malloc'd buffer in BUF.  */
	fclose (f);

#else /* !_LIBC */
	/* Build the message the old bad way.  */

#define LEFT (sizeof tbuf - (p - tbuf))

	(void)time(&now);
	p = tbuf + sprintf(tbuf, "<%d>", pri);
	p += strftime(p, LEFT, "%h %e %T ", localtime(&now));

	msgoff = p - tbuf;

	if (LogTag == NULL)
		LogTag = __progname;
	if (LogTag != NULL)
		p += snprintf(p, LEFT, "%s", LogTag);
	if (LogStat & LOG_PID)
		p += snprintf(p, LEFT, "[%d]", getpid());
	if (LogTag != NULL) {
		*p++ = ':';
		*p++ = ' ';
	}

	/* Substitute error message for %m. */
	for (t = fmt_cpy; ch = *fmt && t < fmt_cpy - 1; ++fmt)
		if (ch == '%' && fmt[1] == 'm') {
			++fmt;
			t += snprintf(t, sizeof fmt_cpy - (t - fmt_cpy),
				      "%s", strerror(saved_errno));
		} else
			*t++ = ch;
	*t = '\0';

	p += vsnprintf(p, LEFT, fmt_cpy, ap);

	buf = tbuf;
	bufsize = p - tbuf;
#endif /* _LIBC */

	/* Output to stderr if requested. */
	if (LogStat & LOG_PERROR) {
		struct iovec iov[2];
		register struct iovec *v = iov;

		v->iov_base = buf + msgoff;
		v->iov_len = bufsize - msgoff;
		++v;
		v->iov_base = (char *) "\n";
		v->iov_len = 1;
		(void)writev(STDERR_FILENO, iov, 2);
	}

#if _LIBC - 0
	/* Prepare for multiple users.  We have to take care: open and
	   write are cancellation points.  */
#if (__GNUC__ >= 3)
	__libc_cleanup_region_start (1, (void (*) (void *)) cancel_handler,
				     &oldaction_ptr);
#else 
	__libc_cleanup_region_start ((void (*) (void *)) cancel_handler,
				     &oldaction_ptr);
#endif /* if __GNUC >= 3 */
	__libc_lock_lock (syslog_lock);
#endif /* if _LIBC */


	/* Prepare for a broken connection.  */
 	memset (&action, 0, sizeof (action));
 	action.sa_handler = sigpipe_handler;
 	sigemptyset (&action.sa_mask);
 	sigpipe = sigaction (SIGPIPE, &action, &oldaction);
	if (sigpipe == 0)
	  oldaction_ptr = &oldaction;

	/* Get connected, output the message to the local logger. */
	if (!connected)
		openlog_internal(LogTag, LogStat | LOG_NDELAY, 0);

	/* If we have a SOCK_STREAM connection, also send ASCII NUL as
	   a record terminator.  */
	if (LogType == SOCK_STREAM)
	  ++bufsize;

	//if (__send(LogFile, buf, bufsize, 0) < 0)
	if (send(LogFile, buf, bufsize, 0) < 0)
	  {
	    closelog_internal ();	/* attempt re-open next time */
	    /*
	     * Output the message to the console; don't worry about blocking,
	     * if console blocks everything will.  Make sure the error reported
	     * is the one from the syslogd failure.
	     */
	    if (LogStat & LOG_CONS &&
		(fd = open(_PATH_CONSOLE, O_WRONLY, 0)) >= 0)
	      {
#if _LIBC - 0
		dprintf (fd, "%s\r\n", buf + msgoff);
#else /* !_LIBC */
		if (bufsize < sizeof tbuf - 3)
		  {
		    strcpy (buf + bufsize, "\r\n");
		    write (fd, buf + msgoff, bufsize - msgoff + 2);
		  }
		else
		  {
		    strcpy (buf + sizeof tbuf - 3, "\r\n");
		    write (fd, buf + msgoff, sizeof tbuf - 1 - msgoff);
		  }
#endif /* _LIBC */

		(void)close(fd);
	      }
	  }

	if (sigpipe == 0)
		sigaction (SIGPIPE, &oldaction, (struct sigaction *) NULL);

#if _LIBC - 0
	/* End of critical section.  */
	__libc_cleanup_region_end (0);
	__libc_lock_unlock (syslog_lock);

	free (buf);
#endif
}

static struct sockaddr SyslogAddr;	/* AF_UNIX address of local logger */

static void
openlog_internal(const char *ident, int logstat, int logfac)
{
	if (ident != NULL)
		LogTag = ident;
	LogStat = logstat;
	if (logfac != 0 && (logfac &~ LOG_FACMASK) == 0)
		LogFacility = logfac;

	while (1) {
		if (LogFile == -1) {
			SyslogAddr.sa_family = AF_UNIX;
			(void)strncpy(SyslogAddr.sa_data, _PATH_LOG,
				      sizeof(SyslogAddr.sa_data));
			if (LogStat & LOG_NDELAY) {
				if ((LogFile = socket(AF_UNIX, LogType, 0))
				    == -1)
					return;
				(void)fcntl(LogFile, F_SETFD, 1);
			}
		}
		if (LogFile != -1 && !connected) {
                    //if (__connect(LogFile, &SyslogAddr, sizeof(SyslogAddr))
                    if (connect(LogFile, &SyslogAddr, sizeof(SyslogAddr))
			    == -1)
			{
				int saved_errno = errno;
				(void)close(LogFile);
				LogFile = -1;
				if (LogType == SOCK_DGRAM
				    && saved_errno == EPROTOTYPE)
				{
					/* retry with next SOCK_STREAM: */
					LogType = SOCK_STREAM;
					continue;
				}
			} else
				connected = 1;
                }
		break;
	}
}

void
openlog (const char *ident, int logstat, int logfac)
{
#if _LIBC - 0
  /* Protect against multiple users.  */
#if (__GNUC__ >= 3)
    __libc_cleanup_region_start (1, (void (*) __P ((void *))) __libc_mutex_unlock,
			       &syslog_lock);
#else
    __libc_cleanup_region_start ((void (*) __P ((void *))) __libc_mutex_unlock,
			       &syslog_lock);
#endif /* if __GNUC__ >= 3 */

  __libc_lock_lock (syslog_lock);
#endif /* if _LIBC */

  openlog_internal (ident, logstat, logfac);

#if _LIBC - 0
  /* Free the lock.  */
  __libc_cleanup_region_end (1);
#endif
}

static void
sigpipe_handler (int signo)
{
	closelog_internal();
}

static void
closelog_internal()
{
	(void)close(LogFile);
	LogFile = -1;
	connected = 0;
}

void
closelog ()
{
#if _LIBC - 0
#if (__GNUC__ >= 3)
    __libc_cleanup_region_start (1, (void (*) __P ((void *))) __libc_mutex_unlock,
			       &syslog_lock);
#else
    __libc_cleanup_region_start ((void (*) __P ((void *))) __libc_mutex_unlock,
			       &syslog_lock);
#endif /* if __GNUC__ >= 3 */

  __libc_lock_lock (syslog_lock);
#endif /* if _LIBC */

  closelog_internal ();

#if _LIBC - 0
  /* Free the lock.  */
  __libc_cleanup_region_end (1);
#endif
}

#if _LIBC - 0 && defined (_LIBC_REENTRANT)
static void
cancel_handler (void *ptr)
{
  /* Restore the old signal handler.  */
  struct sigaction *oldaction = *((struct sigaction **) ptr);

  if (oldaction != (struct sigaction *) NULL)
    sigaction (SIGPIPE, oldaction, (struct sigaction *) NULL);

  /* Free the lock.  */
  __libc_lock_unlock (syslog_lock);
}
#endif /* _LIBC - 0 && defined (_LIBC_REENTRANT) */

/* setlogmask -- set the log mask level */
int
setlogmask(pmask)
	int pmask;
{
	int omask;

	omask = LogMask;
	if (pmask != 0)
		LogMask = pmask;
	return (omask);
}
