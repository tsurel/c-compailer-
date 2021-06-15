CC = gcc
CFLAGS = -Wall -ansi -pedantic

assembler: assembler.o functions.o symboltable.o operations.o utils.o
	$(CC) $(CFLAGS) assembler.o functions.o symboltable.o operations.o utils.o -o assembler

assembler.o: assembler.c functions.h
	$(CC) -c $(CFLAGS) assembler.c -o assembler.o

functions.o: functions.c functions.h symboltable.h errorhandler.h utils.h
	$(CC) -c $(CFLAGS) functions.c -o functions.o

symboltable.o: symboltable.c symboltable.h errorhandler.h
	$(CC) -c $(CFLAGS) symboltable.c -o symboltable.o

operations.o: operations.c operations.h errorhandler.h
	$(CC) -c $(CFLAGS) operations.c -o operations.o

#errorhandler.o: errorhandler.c errorhandler.h
#	$(CC) -c $(CFLAGS) errorhandler.c -o errorhandler.o

utils.o: utils.c utils.h
	$(CC) -c $(CFLAGS) utils.c -o utils.o

clean:
	rm -f *.o assembler
