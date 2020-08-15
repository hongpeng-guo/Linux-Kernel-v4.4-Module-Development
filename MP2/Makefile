EXTRA_CFLAGS +=
APP_EXTRA_FLAGS:= -O2 -ansi -pedantic
KERNEL_SRC:= /lib/modules/$(shell uname -r)/build
SUBDIR= $(PWD)
GCC:=gcc
RM:=rm

.PHONY : clean

all: clean modules app

obj-m:= mp2.o
mp2-objs:= mp2_main.o scheduler.o

modules:
	$(MAKE) -C $(KERNEL_SRC) M=$(SUBDIR) modules
	mv mp2.ko hg5_MP2.ko
	
app: userapp.c userapp.h
	$(GCC) -o userapp userapp.c

clean:
	$(RM) -f userapp *~ *.ko *.o *.mod.c Module.symvers modules.order
