EXTRA_CFLAGS +=
APP_EXTRA_FLAGS:= -O2 -ansi -pedantic
KERNEL_SRC:= /lib/modules/$(shell uname -r)/build
SUBDIR= $(PWD)
GCC:=gcc
RM:=rm

.PHONY : clean

all: clean modules app

obj-m:= mp3.o
mp2-objs:= mp2_main.o profiler.o

modules:
	$(MAKE) -C $(KERNEL_SRC) M=$(SUBDIR) modules
#	mv mp2.ko hg5_MP2.ko
	
app: work.c monitor.h
	$(GCC) -o work work.c
	$(GCC) -o monitor monitor.c

clean:
	$(RM) -f work monitor *~ *.ko *.o *.mod.c Module.symvers modules.order
