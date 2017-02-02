obj-m := hello.o
KVERSION = $(shell uname -r)
KDIR := /lib/modules/$(KVERSION)/build
MAKE = make

all:
        make -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
        make -C $(KDIR) SUBDIRS=$(PWD) clean
