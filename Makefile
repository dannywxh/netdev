.PHONY:clean all

CC=gcc

CFLAGS=-Wall -g

BIN=echo_srv echo_cli  

all:$(BIN) 

%.o:%.c #所有的c生成o
		$(CC) $(CFLAGS) -c $< -o $@
		
#echo_cli 如果只依赖与echo_cli.o,make可以进行隐含推导,不需要明确的写出
#echo_cli:echo_cli.o pub.o	
echo_srv:echo_srv.o pub.o		

clean:
		rm -f *.o $(BIN)		 