EXTRA_CFLAGS +=
APP_EXTRA_FLAGS:= -O2 -ansi -pedantic
KERNEL_SRC:= /lib/modules/$(shell uname -r)/build
SUBDIR= $(PWD)
GCC:=gcc
RM:=rm

.PHONY : clean

all: clean modules app

obj-m:= mp3.o
mp3-objs:= mp3_main.o profiler.o

modules:
	$(MAKE) -C $(KERNEL_SRC) M=$(SUBDIR) modules	
	mv mp3.ko hg5_MP3.ko
	
app: work.c monitor.c
	$(GCC) -o work work.c
	$(GCC) -o monitor monitor.c

clean:
	$(RM) -f work monitor *~ *.ko *.o *.mod.c Module.symvers modules.order
