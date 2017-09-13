.SUFFIXES: .c .o

CC=gcc

SRCS1=echo_cli.c
SRCS2=echo_srv.c	
PUB=pub.c

OBJS1=$(SRCS1:.c=.o)
OBJS2=$(SRCS2:.c=.o)
OBJPUB=$(PUB:.c=.o)

BIN1=echo_cli
BIN2=echo_srv

all: $(OBJS1) $(OBJS2) $(OBJPUB)
	$(CC) -o $(BIN1) $(OBJS1) $(OBJPUB) -lpthread
	$(CC) -o $(BIN2) $(OBJS2) $(OBJPUB) -lpthread
	
	@echo 'Build... ^_^ ^_^ ^_^ ^_^ ^_^ ^_^ OK ^_^ ^_^ ^_^ ^_^ ^_^ ^_^'

.c.o: 
	$(CC) -Wall -g -o $@ -c $<

clean:
	-rm -f *.o $(BIN1) $(BIN2)
	-rm -f core*

