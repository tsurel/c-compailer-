assembler: assembler.o functions.o
	gcc -Wall -ansi -pedantic assembler.o functions.o -o assembler

assembler.o: assembler.c functions.h
	gcc -c -Wall -ansi -pedantic assembler.c -o assembler.o

functions.o: functions.c functions.h
	gcc -c -Wall -ansi -pedantic functions.c -o functions.o