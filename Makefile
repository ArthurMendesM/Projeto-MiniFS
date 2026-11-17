CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g

OBJS_PRINCIPAL = main.o disco.o inode.o arquivos.o

all: MiniFS

MiniFS: $(OBJS_PRINCIPAL)
	$(CC) $(CFLAGS) -o MiniFS $(OBJS_PRINCIPAL)

main.o: main.c disco.h inode.h arquivos.h
	$(CC) $(CFLAGS) -c main.c

disco.o: disco.c disco.h
	$(CC) $(CFLAGS) -c disco.c

inode.o: inode.c inode.h disco.h
	$(CC) $(CFLAGS) -c inode.c

arquivos.o: arquivos.c arquivos.h disco.h inode.h
	$(CC) $(CFLAGS) -c arquivos.c

testes:
	$(CC) $(CFLAGS) -o teste_est1 mainTesteEstudante1.c disco.c
	$(CC) $(CFLAGS) -o teste_est2 "main TesteEstudante2.c" disco.c inode.c

clean:
	rm -f *.o MiniFS teste_est1 teste_est2

.PHONY: all testes clean
