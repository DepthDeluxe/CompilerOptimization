#include <glib.h>

#include "compiler.h"

void printNodeType(TreeNode* node);
void printNode(TreeNode* node_ptr, GList* parent_stack);

// Code Generation support functions
int  emitSkip(int howMany);
void emitBackup(int loc);
void emitRestore(void);
void emitComment(char*  c);
void emitRO(char* op, int r, int s, int t, char* c);
void emitRM(char* op, int r, int d, int s, char* c);
void emitFloatSet(int reg, float value);
void emitRMAbs( char* op, int r, int a, char*  c);
void emitOppositeSelStmt(int relop, int reg, int loc);
void push(int reg, char* comm) ;
void pop(int reg, char* comm);

