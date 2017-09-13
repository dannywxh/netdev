.PHONY:clean all

CC=gcc

CFLAGS=-Wall -g

BIN=echo_srv echo_cli  

all:$(BIN) 

%.o:%.c 
		$(CC) $(CFLAGS) -c $< -o $@
		
%.o:%.cpp 
		g++ $(CFLAGS) -c $< -o $@
		
echo_cli:echo_cli.o pub.o	
		g++ $(CFLAGS) $^ -o $@   
echo_srv:echo_srv.o pub.o		
		g++ $(CFLAGS) $^ -o $@        

clean:
		rm -f *.o $(BIN)		 