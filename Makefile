.PHONY:clean all

CC=gcc

CFLAGS=-Wall -g

BIN=echo_srv echo_cli  

all:$(BIN) 

%.o:%.c #���е�c����o
		$(CC) $(CFLAGS) -c $< -o $@
		
#echo_cli ���ֻ������echo_cli.o,make���Խ��������Ƶ�,����Ҫ��ȷ��д��
#echo_cli:echo_cli.o pub.o	
echo_srv:echo_srv.o pub.o		

clean:
		rm -f *.o $(BIN)		 