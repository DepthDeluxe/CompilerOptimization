#ifndef _CODEGEN_SUPPORT_H
#define _CODEGEN_SUPPORT_H

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

void codegenSupportInit();
void writeInstruction(int loc, TMInstruction i);
TMInstruction* lookupInstruction(int label);
int getASMPrintType(Opcode op);
void printOpcode(Opcode op);
void printInstruction(int label, TMInstruction* i);
void printInstructionTable();


#endif

