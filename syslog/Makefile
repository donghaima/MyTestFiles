# Simple makefile
# 
CC=gcc

CFLAGS=-Wall -D_LIBC_REENTRANT

all: test_syslog test_libc_syslog 

SRC = test_syslog.c syslog.c
OBJS = $(SRC:.c=.o)

test_syslog: $(OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@ $(LIBS)

test_libc_syslog: test_syslog.o
	$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@ $(LIBS)


clean:
	rm -rf $(OBJS) test_syslog



