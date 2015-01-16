CC = gcc

CFLAGS =

all: cm

lex.yy.c: scan.lex compiler.h
	flex scan.lex

y.tab.c: parse.yac lex.yy.c compiler.h
	yacc parse.yac

y.tab.o: y.tab.c
	$(CC) $(CFLAGS) -c y.tab.c

semantic.o: compiler.h semantic.c
	$(CC) $(CFLAGS) -c semantic.c

codegen.o: compiler.h codegen.c
	$(CC) $(CFLAGS) -c codegen.c

main.o: compiler.h main.c
	$(CC) $(CFLAGS) -c main.c

cm: y.tab.o main.o semantic.o codegen.o
	$(CC) $(CFLAGS) main.o semantic.o codegen.o y.tab.o -o cm -ly

clean:
	-rm cm lex.yy.c y.tab.c *.o
