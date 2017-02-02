/*
 * grab: move a process from one tty to another.
 *   Version 0.0.1
 *
 * Written by Bernard Blackham. This code is in the public domain.
 *
 * This is a very quick hack and quite a mess. Needs a lot of polish. It is
 * also nowhere near perfect.
 *
 * Usage:
 *        some tty: $ vim
 *     another tty: ./grab <pid of vim>
 *        some tty: [1]+  Stopped                 vim
 *                  $ bg
 *                  $ disown
 *     another tty: ^L
 *                  et voila, a vim.
 */

#include <linux/user.h>
#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <asm/termios.h>
#include <asm/unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

pid_t pid;
void *target_page = (void*)(0x08048000);
void *backup_page;
long steal_size = (1<<12)*2; /* 2 pages == 8K */

char tty[64];

int memcpy_from_target(pid_t pid, void* dest, const void* src, size_t n) {
    /* just like memcpy, but copies it from the space of the target pid */
    /* n must be a multiple of 4, or will otherwise be rounded down to be so */
    int i;
    long *d, *s;
    d = (long*) dest;
    s = (long*) src;
    n /= sizeof(long);
    for (i = 0; i < n; i++) {
	d[i] = ptrace(PTRACE_PEEKTEXT, pid, s+i, 0);
	if (errno) {
	    perror("ptrace(PTRACE_PEEKTEXT)");
	    return 0;
	}
    }
    return 1;
}

int memcpy_into_target(pid_t pid, void* dest, const void* src, size_t n) {
    /* just like memcpy, but copies it into the space of the target pid */
    /* n must be a multiple of 4, or will otherwise be rounded down to be so */
    int i;
    long *d, *s;
    d = (long*) dest;
    s = (long*) src;
    for (i = 0; i < n / sizeof(long); i++) {
	if (ptrace(PTRACE_POKETEXT, pid, d+i, s[i]) == -1) {
	    perror("ptrace(PTRACE_POKETEXT)");
	    return 0;
	}
    }
    return 1;
}

int do_syscall(pid_t pid, struct user_regs_struct *regs) {
    long loc;
    struct user_regs_struct orig_regs;
    long old_insn;
    int status, ret;

    if (ptrace(PTRACE_GETREGS, pid, NULL, &orig_regs) < 0) {
	perror("ptrace getregs");
	return 0;
    }

    loc = (long)target_page;

    old_insn = ptrace(PTRACE_PEEKTEXT, pid, loc, 0);
    if (errno) {
	perror("ptrace peektext");
	return 0;
    }
    //printf("original instruction at 0x%lx was 0x%lx\n", loc, old_insn);

    if (ptrace(PTRACE_POKETEXT, pid, loc, 0x80cd) < 0) {
	perror("ptrace poketext");
	return 0;
    }

    /* Set up registers for ptrace syscall */
    regs->eip = loc;
    if (ptrace(PTRACE_SETREGS, pid, NULL, regs) < 0) {
	perror("ptrace setregs");
	return 0;
    }

    /* Execute call */
    if (ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL) < 0) {
	perror("ptrace singlestep");
	return 0;
    }
    ret = waitpid(pid, &status, 0);
    if (ret == -1) {
	perror("Failed to wait for child");
	exit(1);
    }

    /* Get our new registers */
    if (ptrace(PTRACE_GETREGS, pid, NULL, regs) < 0) {
	perror("ptrace getregs");
	return 0;
    }

    /* Return everything back to normal */
    if (ptrace(PTRACE_SETREGS, pid, NULL, &orig_regs) < 0) {
	perror("ptrace getregs");
	return 0;
    }

    if (ptrace(PTRACE_POKETEXT, pid, loc, old_insn) < 0) {
	perror("ptrace poketext");
	return 0;
    }

    return 1;
}

int get_termios(pid_t pid, int fd, struct termios *t) {
    struct user_regs_struct r;

    if (ptrace(PTRACE_GETREGS, pid, 0, &r) == -1) {
        perror("ptrace(GETREGS)");
        return 0;
    }

    r.eax = __NR_ioctl;
    r.ebx = fd;
    r.ecx = TCGETS;
    r.edx = ((long)target_page)+0x10;

    if (!do_syscall(pid, &r)) return 0;

    /* Error checking! */
    if (r.eax < 0) {
        errno = -r.eax;
        perror("target ioctl");
        return 0;
    }

    memcpy_from_target(pid, t, (void*)(target_page+0x50), sizeof(struct termios));

    return 1;
}

int grab(pid_t pid) {
    FILE *f;
    int fd, status;
    char tmp_fn[1024], stat_line[1024], fd_filename[1024], *stat_ptr;
    struct user_regs_struct orig_regs, new_regs;
    struct termios t;
    struct stat stat_buf;
    struct dirent *fd_dirent;
    int have_termios = 0;
    int term_dev;

    /* Get the process's terminal device */
    /* This involves parsing /proc/stat :( */
    term_dev = 0;
    snprintf(tmp_fn, 1024, "/proc/%d/stat", pid);
    memset(stat_line, 0, sizeof(stat_line));
    f = fopen(tmp_fn, "r");
    fgets(stat_line, 1024, f);
    fclose(f);
    stat_ptr = strrchr(stat_line, ')');
    if (stat_ptr != NULL) {
	int tty = -1;

	stat_ptr += 2;
	sscanf(stat_ptr, "%*c %*d %*d %*d %d", &tty);
	if (tty > 0) {
	    term_dev = (dev_t)tty;
	}
    }

    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
        perror("ptrace(PTRACE_ATTACH)");
        exit(1);
    }
    if (wait(&status) == -1) {
        perror("wait()");
        exit(1);
    }
    if (!WIFSTOPPED(status)) {
        fprintf(stderr, "Failed to get stopped child!\n");
    }

    /* Backup pages */
    backup_page = malloc(steal_size);
    if (memcpy_from_target(pid, backup_page, target_page, steal_size) == 0)
        exit(1);

    if (ptrace(PTRACE_GETREGS, pid, NULL, &orig_regs) == -1) {
        perror("ptrace(PTRACE_GETREGS)");
        exit(1);
    }

    snprintf(tmp_fn, 1024, "/proc/%d/fd", pid);
    DIR *proc_fd = opendir(tmp_fn);
    int mode;
    for (;;) {
	fd_dirent = readdir(proc_fd);
	if (fd_dirent == NULL)
	    break;
	if (fd_dirent->d_type != DT_LNK)
	    continue;

	snprintf(tmp_fn, 1024, "/proc/%d/fd/%s", pid, fd_dirent->d_name);
	lstat(tmp_fn, &stat_buf);
	if ((stat_buf.st_mode & S_IRUSR) && (stat_buf.st_mode & S_IWUSR))
	    mode = O_RDWR;
	else if (stat_buf.st_mode & S_IWUSR)
	    mode = O_WRONLY;
	else
	    mode = O_RDONLY;

	/* work out what file this FD points to */
	memset(fd_filename, 0, sizeof(fd_filename));
	readlink(tmp_fn, fd_filename, sizeof(fd_filename)-1);

        fd = atoi(fd_dirent->d_name);
	/* stat the file this time, not the link */
	if (stat(tmp_fn, &stat_buf) < 0)
	    continue;
	if (!S_ISCHR(stat_buf.st_mode) || !(stat_buf.st_rdev == term_dev))
            continue;


        if (!have_termios) {
            get_termios(pid, fd, &t);
            have_termios = 1;
        }


        /* Open a new FD with this terminal */
        memcpy_into_target(pid, target_page+0x50, tty, strlen(tty)+4);
        memcpy(&new_regs, &orig_regs, sizeof(new_regs));
        new_regs.eax = __NR_open;
        new_regs.ebx = (long)target_page+0x50;
        new_regs.ecx = O_RDWR;
        new_regs.edx = 0755;
        do_syscall(pid, &new_regs);

        int new_fd = new_regs.eax;
        fprintf(stderr, "New FD is %d\n", new_fd);

        /* Dup it over the relevant FD */
        memcpy(&new_regs, &orig_regs, sizeof(new_regs));
        new_regs.eax = __NR_dup2;
        new_regs.ebx = new_fd;
        new_regs.ecx = fd;
        do_syscall(pid, &new_regs);

        /* Close the old FD */
        memcpy(&new_regs, &orig_regs, sizeof(new_regs));
        new_regs.eax = __NR_close;
        new_regs.ebx = new_fd;
        do_syscall(pid, &new_regs);

    }
    closedir(proc_fd);

    /* Alter it's controlling TTY */
    int pgrp = getpgrp();
    memcpy(&new_regs, &orig_regs, sizeof(new_regs));
    memcpy_into_target(pid, target_page+0x50, &getpgrp, 4);
    new_regs.eax = __NR_ioctl;
    new_regs.ebx = fd-1;
    new_regs.ecx = TIOCSPGRP;
    new_regs.edx = (long)(target_page+0x50);
    do_syscall(pid, &new_regs);

    /* Restore pages */
    if (memcpy_into_target(pid, target_page, backup_page, steal_size) == 0)
        exit(1);
    if (ptrace(PTRACE_SETREGS, pid, NULL, &orig_regs) == 1) {
            perror("ptrace(PTRACE_SETREGS)");
        exit(1);
    }
    if (ptrace(PTRACE_DETACH, pid, NULL, NULL) == 1) {
        perror("ptrace(PTRACE_DETACH)");
        exit(1);
    }
    ioctl(0, TCSETS, &t);
}

int main(int argc, char** argv) {
    struct termios orig_tios;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
        return 1;
    }
    pid = atoi(argv[1]);
    if (kill(pid, 0) == -1) {
        fprintf(stderr, "Cannot grab process: %s\n", strerror(errno));
        return 1;
    }

    char fd0path[64];
    snprintf(fd0path, 64, "/proc/%d/fd/0", getpid());
    if (readlink(fd0path, tty, 64) == -1) {
        perror("readlink");
        exit(1);
    }

    ioctl(0, TCGETS, &orig_tios);

    grab(pid);

    if (ioctl(0, TIOCSPGRP, pid) == -1) {
        perror("ioctl(TIOCSPGRP)");
    }

    close(0);
    close(1);
    close(2);

    /* Let the user background the process and disown it elsewhere */
    kill(pid, SIGSTOP);

    /* We're not the child's parent, so we have to kill polling until it's
     * dead :(
     */
    while (kill(pid, 0) == 0) { sleep(1); }

    ioctl(0, TCSETS, &orig_tios);
    return 0;
}
