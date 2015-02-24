#include <glib.h>

#include "compiler.h"

void printNodeType(TreeNode* node);
void printNode(TreeNode* node_ptr, GList* parent_stack);

// Semantic checking support functions
extern SemRec*    lookup(int line, SymbolTable scopePtr, char* theName);
extern void       insert(int line, SymbolTable scopePtr, char* theName, SemRec* theRec);
extern void       beginScope();
extern void       endScope();

extern void       upScope();
extern void       downScope();

extern char*      myalloc(size_t size);
extern TreeNode*  newParseTreeNode();
extern SemRec*    newSemRec();
extern HashNode*  newHTableNode();
extern HashTable* newHashTable();
extern Scope*     newScope();

// Semantic checker support functions
extern void semanticCheck(TreeNode* nodePtr);

// TM Instruction generation support functions
typedef enum {
  HALT, IN, INF, OUT, OUTF, LD, ST, LDA, LDC, LDF,
  ADD, SUB, MUL, DIV,
  JEQ, JNE, JLT, JLE, JGT, JGE
} Opcode;

// Code Generation support functions
int  emitSkip(int howMany);
void emitBackup(int loc);
void emitRestore(void);
void emitRO(Opcode op, int r, int s, int t, char* c);
void emitRM(Opcode op, int r, int d, int s, char* c);
void emitFloatSet(int reg, float value);
void emitRMAbs( Opcode op, int r, int a, char*  c);
void emitOppositeSelStmt(int relop, int reg, int loc);
void push(int reg, char* comm) ;
void pop(int reg, char* comm);

typedef struct tminstruction {
  Opcode opCode;
  int a,b,c;
  char* comment;
} TMInstruction;

void codegenSupportInit();
void writeInstruction(int loc, TMInstruction i);
TMInstruction* lookupInstruction(int label);
int getASMPrintType(Opcode op);
void printOpcode(Opcode op);
void printInstruction(int label, TMInstruction* i);
void printInstructionTable();

// code profiler and optimizer
void profileLDST();
void profileSTLD();
void profileADDSUB();
void profileMULDIV();
void profileDIVMUL();
void profileLDNULL();
void profileDUP();

void profile();
