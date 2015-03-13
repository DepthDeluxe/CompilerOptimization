#ifndef _COMPILER_H
#define _COMPILER_H

#include <glib.h>

#include "types.h"

#define pc  7            // Program Counter (next instruction addr)
#define sp  6            // Stack Pointer (top of stack)
#define fp  5            // Frame Pointer (top of current frame in stack)
#define gp  4            // Globals Pointer (top of program)
#define ac1 1            // Accumulator 1
#define ac0 0            // Accumulator 0 (return values go here)

#define YYSTYPE ParseTree    // The parser forms a parse tree

#define M 11

// options
int with_float;

extern ParseTree     parseTreePtr;
extern int           lineno;

// hash table to store instructions
extern GHashTable* instructionTable;

// For debugging purposes
#define _test(a,b)  if (NULL==a) fprintf(stderr, "ACCESSING NULL %s\n", b);

#endif
