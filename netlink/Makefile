# A simple Makefile

CC = gcc
CFLAGS = -g

OBJS = test_iproute_get.o libnetlink.o utils.o ll_map.o

test_iproute_get: ${OBJS}
	${CC} -o test_iproute_get ${OBJS}

test_iproute_get.o: test_iproute_get.c libnetlink.h utils.h
	${CC} ${CFLAGS} -c test_iproute_get.c

libnetlink.o: libnetlink.c libnetlink.h
	${CC} ${CFLAGS} -c libnetlink.c

utils.o: utils.c utils.h
	${CC} ${CFLAGS} -c utils.c

ll_map.o: ll_map.c ll_map.h
	${CC} ${CFLAGS} -c ll_map.c

rtm_map.o: rtm_map.c rtm_map.h
	${CC} ${CFLAGS} -c rtm_map.c

clean:
	rm -f test_iproute_get ${OBJS}
	@echo "all cleaned up!"

