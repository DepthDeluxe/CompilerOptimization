# compile compiler with GCC
CLANG_PATH = "/usr/bin/clang"
ifneq ($(wildcard $(CLANG_PATH)),"")
CC = clang
else
CC = gcc
endif

# import glib into build system
GLIB = `pkg-config --cflags --libs glib-2.0`
CFLAGS = -std=c99 -Wall $(GLIB)

# main build command
all: ctags cm

gcc: CC=gcc
gcc: all
debug: CFLAGS += -g
debug: all

lex.yy.c: scan.lex compiler.h
	flex scan.lex

y.tab.c: parse.yac lex.yy.c compiler.h
	yacc parse.yac

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

cm: y.tab.o main.o semantic.o semantic_support.o codegen.o codegen_support.o support.o profiler.o
	$(CC) $(CFLAGS) y.tab.o main.o semantic.o semantic_support.o codegen.o codegen_support.o support.o profiler.o -o cm -ly

ctags:
	ctags *

clean:
	rm -f cm lex.yy.c y.tab.c *.o tags
