CC = gcc
CFLAGS = -Wall -ansi -pedantic

assembler: assembler.o converter.o symboltable.o keywords.o asmutils.o errmsg.o utils.o
	$(CC) $(CFLAGS) assembler.o converter.o symboltable.o keywords.o asmutils.o errmsg.o utils.o -o assembler

assembler.o: assembler.c converter.h
	$(CC) -c $(CFLAGS) assembler.c -o assembler.o

converter.o: converter.c converter.h symboltable.h keywords.h asmutils.h utils.h
	$(CC) -c $(CFLAGS) converter.c -o converter.o

symboltable.o: symboltable.c symboltable.h asmutils.h
	$(CC) -c $(CFLAGS) symboltable.c -o symboltable.o

keywords.o: keywords.c keywords.h asmutils.h
	$(CC) -c $(CFLAGS) keywords.c -o keywords.o

asmutils.o: asmutils.c asmutils.h utils.h
	$(CC) -c $(CFLAGS) asmutils.c -o asmutils.o

errmsg.o: errmsg.c errmsg.h asmutils.h
	$(CC) -c $(CFLAGS) errmsg.c -o errmsg.o

utils.o: utils.c utils.h
	$(CC) -c $(CFLAGS) utils.c -o utils.o

clean:
	rm -f *.o assembler
