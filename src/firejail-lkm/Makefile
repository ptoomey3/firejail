# install linux-headers on Debian/Ubuntu:
# apt-get install linux-headers-$(uname -r)


VERSION = 0.1
KDIR  ?= /lib/modules/$(shell uname -r)/build
#KDIR = /usr/kernelsrc/linux-source-3.14

firejail-objs := main.o split.o syscall.o timeout.o uptime.o utils.o
obj-m = firejail.o
all: firejail.ko

firejail.ko: firejail.h main.c split.c syscall.c timeout.c uptime.c utils.c
	make -C $(KDIR) M=$(CURDIR) modules

clean:
	make -C $(KDIR) M=$(CURDIR) clean
	rm -f *.so *.o modules.order .*cmd
	rm -f tmon

start: all
	insmod firejail.ko
	sleep 2; cat /proc/firejail
	
stop:
	rmmod firejail

dist:
	rm -fr firejail-lkm
	mkdir -p firejail-lkm
	cp Makefile firejail-lkm/.
	cp *.c firejail-lkm/.
	cp *.h firejail-lkm/.
	tar -cjvf firejail-lkm-$(VERSION).tar.bz2 firejail-lkm
	rm -fr firejail-lkm
