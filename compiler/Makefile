# compile compiler with GCC
CLANG_PATH = "/usr/bin/clang"
ifneq ($(wildcard $(CLANG_PATH)),"")
CC = clang
else
CC = gcc
endif

# import glib into build system
CFLAGS = -std=c99 -Wall `pkg-config --cflags glib-2.0`
LFLAGS = $(CFLAGS) `pkg-config --libs glib-2.0`

SOURCES := y.tab.c main.c semantic.c semantic_support.c codegen.o codegen_support.c support.c profiler.c trimmer.c

# main build command
all: ctags cm

gcc: CC=gcc
gcc: all
debug: CFLAGS += -g
debug: all

# import the dependency files
-include $(SOURCES=.c=.d)

%.o: %.c %.h
	$(info Buliding: $<)
	@$(CC) $(CFLAGS) -c $<
	@$(CC) $(CFLAGS) -MM $< -o $(@:.o=.d)

# generate lexer and parser
lex.yy.c: scan.lex compiler.h
	$(info Building lexer)
	@flex scan.lex
y.tab.c: parse.yac lex.yy.c compiler.h
	$(info Building parser)
	@yacc parse.yac

cm: $(SOURCES:.c=.o)
	$(info Linking program)
	@$(CC) $(LFLAGS) $? -o cm

ctags:
	$(info Building ctags file)
	@ctags *

clean:
	rm -f cm lex.yy.c y.tab.c *.o *.d tags
