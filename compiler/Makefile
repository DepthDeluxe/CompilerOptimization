# compile compiler with GCC
CC = gcc

# import glib into build system
CFLAGS = -std=c99 -Wall `pkg-config --cflags --libs glib-2.0`

# main build command
all: ctags cm

debug: CFLAGS += -g
debug: ctags cm

lex.yy.c: scan.lex compiler.h
	flex scan.lex

y.tab.c: parse.yac lex.yy.c compiler.h
	yacc parse.yac

y.tab.o: y.tab.c
	$(CC) $(CFLAGS) -c y.tab.c

semantic.o: compiler.h semantic.c
	$(CC) $(CFLAGS) -c semantic.c

support.o: compiler.h support.c
	$(CC) $(CFLAGS) -c support.c

codegen.o: compiler.h codegen.c
	$(CC) $(CFLAGS) -c codegen.c

main.o: compiler.h main.c
	$(CC) $(CFLAGS) -c main.c

cm: y.tab.o main.o semantic.o support.o codegen.o
	$(CC) $(CFLAGS) main.o semantic.o support.o codegen.o y.tab.o -o cm -ly

ctags:
	ctags *

clean:
	-rm cm lex.yy.c y.tab.c *.o tags
