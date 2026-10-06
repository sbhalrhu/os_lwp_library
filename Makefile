CC 	= gcc
AR 	= ar

CFLAGS  = -Wall -g -I .

LD 	= gcc

LDFLAGS = -Wall -g

PROGS	= snakes nums hungry

SNAKEOBJS  = randomsnakes.o util.o
HUNGRYOBJS = hungrysnakes.o util.o
NUMOBJS    = numbersmain.o

OBJS	= $(SNAKEOBJS) $(HUNGRYOBJS) $(NUMOBJS)

SRCS	= lwp.c rr.c magic64.S util.c randomsnakes.c numbersmain.c hungrysnakes.c
HDRS	= lwp.h rr.h snakes.h util.h schedulers.h fp.h

EXTRACLEAN = core $(PROGS) liblwp.a

.PHONY: all demos allclean clean

all: 	liblwp.a

demos: 	$(PROGS)

allclean: clean
	@rm -f $(EXTRACLEAN)

clean:
	rm -f $(OBJS) lwp.o rr.o magic64.o *~ TAGS

snakes: $(SNAKEOBJS) liblwp.a libsnakes.so
	$(LD) $(LDFLAGS) -o snakes $(SNAKEOBJS) -L. -Wl,--no-as-needed -lsnakes -lncurses -lrt -Wl,--as-needed -llwp

hungry: $(HUNGRYOBJS) liblwp.a libsnakes.so
	$(LD) $(LDFLAGS) -o hungry $(HUNGRYOBJS) -L. -Wl,--no-as-needed -lsnakes -lncurses -lrt -Wl,--as-needed -llwp

nums: $(NUMOBJS) liblwp.a
	$(LD) $(LDFLAGS) -o nums $(NUMOBJS) -L. -llwp

hungrysnakes.o: hungrysnakes.c lwp.h snakes.h util.h
	$(CC) $(CFLAGS) -c hungrysnakes.c

randomsnakes.o: randomsnakes.c lwp.h snakes.h util.h
	$(CC) $(CFLAGS) -c randomsnakes.c

numbersmain.o: numbersmain.c lwp.h
	$(CC) $(CFLAGS) -c numbersmain.c

util.o: util.c lwp.h snakes.h util.h schedulers.h
	$(CC) $(CFLAGS) -c util.c

lwp.o: lwp.c lwp.h rr.h fp.h
	$(CC) $(CFLAGS) -c lwp.c

rr.o: rr.c lwp.h rr.h
	$(CC) $(CFLAGS) -c rr.c

magic64.o: magic64.S
	$(CC) -c magic64.S

liblwp.a: lwp.o rr.o magic64.o
	$(AR) rcs liblwp.a lwp.o rr.o magic64.o

submission: $(SRCS) $(HDRS) Makefile README.txt
	tar -cf project2_submission.tar $(SRCS) $(HDRS) Makefile README.txt
	gzip -f project2_submission.tar
