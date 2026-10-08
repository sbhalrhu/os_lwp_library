CC 	= gcc

CFLAGS  = -Wall -g -I .

LD 	= gcc

LDFLAGS  = -Wall -g 

PROGS	= snakes nums hungry

SNAKEOBJS  = randomsnakes.o util.o

HUNGRYOBJS = hungrysnakes.o util.o

NUMOBJS    = numbersmain.o

OBJS	= $(SNAKEOBJS) $(HUNGRYOBJS) $(NUMOBJS) 

SRCS	= randomsnakes.c numbersmain.c hungrysnakes.c

HDRS	= 

EXTRACLEAN = core $(PROGS)

all: 	$(PROGS)

clean:
	rm -f $(OBJS) lwp.o rr.o magic64.o liblwp.a $(PROGS) *~ TAGS

snakes: $(SNAKEOBJS) liblwp.a libsnakes.a
	$(LD) $(LDFLAGS) -o snakes $(SNAKEOBJS) -L. -llwp -lsnakes -lncurses -lrt

hungry: $(HUNGRYOBJS) liblwp.a libsnakes.a
	$(LD) $(LDFLAGS) -o hungry $(HUNGRYOBJS) -L. -llwp -lsnakes -lncurses -lrt

nums: numbersmain.o liblwp.a 
	$(LD) $(LDFLAGS) -o nums numbersmain.o -L. -llwp

hungrysnakes.o: lwp.h snakes.h

randomsnakes.o: lwp.h snakes.h

numbersmain.o: lwp.h

liblwp.a: lwp.c rr.c util.c magic64.S
	$(CC) $(CFLAGS) -c rr.c util.c lwp.c magic64.S
	ar rcs liblwp.a util.o lwp.o rr.o magic64.o
	rm lwp.o

submission: lwp.c rr.c util.c Makefile README
	tar -cf project2_submission.tar lwp.c rr.c Makefile README
	gzip project2_submission.tar
