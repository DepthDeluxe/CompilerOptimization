#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "compiler.h"

#include "codegen_support.h"

// From Louden "Compiler Construction".

/* TM location number for current instruction emission */
static int emitLoc = 0;

/* Highest TM location emitted so far
   For use in conjunction with emitSkip,
   emitBackup, and emitRestore */
static int highEmitLoc = 0;

/* Function emitSkip skips "howMany" code
 * locations for later backpatch. It also
 * returns the current code position
 */
int emitSkip(int howMany){
 int i = emitLoc;
 emitLoc += howMany;
 if (highEmitLoc < emitLoc)  highEmitLoc = emitLoc;
 return i;
} /* emitSkip */

/* Procedure emitBackup backs up to
 * loc = a previously skipped location
 */
void emitBackup(int loc){
  if (loc > highEmitLoc) fprintf(stderr, "BUG in emitBackup\n");
  emitLoc = loc;
} /* emitBackup */

/* Procedure emitRestore restores the current
 * code position to the highest previously
 * unemitted position
 */
void emitRestore() { emitLoc = highEmitLoc; }

/* Procedure emitRO emits a register-only
 * TM instruction
 * op = the opcode
 * r = target register
 * s = 1st source register
 * t = 2nd source register
 * c = a comment to be printed if TraceCode is TRUE
 */
void emitRO( Opcode op, int r, int s, int t, char *c){
  TMInstruction i = {op, r, s, t, c};
  writeInstruction(emitLoc++, i);

  if (highEmitLoc < emitLoc) highEmitLoc = emitLoc;
} /* emitRO */

/* Procedure emitRM emits a register-to-memory TM instruction
 * op = the opcode
 * r = target register
 * d = the offset
 * s = the base register
 * c = a comment to be printed if TraceCode is TRUE
 */
void emitRM( Opcode op, int r, int d, int s, char *c){
  TMInstruction i = {op, r, d, s, c};
  writeInstruction(emitLoc++, i);

  if (highEmitLoc < emitLoc)  highEmitLoc = emitLoc;
} /* emitRM */

void emitFloatSet(int reg, float value) {
  TMInstruction i = {LDF, reg, value, 0, " Floating point set"};
  writeInstruction(emitLoc++, i);
}

/* Procedure emitRM_Abs converts an absolute reference
 * to a pc-relative reference when emitting a
 * register-to-memory TM instruction
 * op = the opcode
 * r = target register
 * a = the absolute location in memory
 * c = a comment to be printed if TraceCode is TRUE
 */
void emitRMAbs( Opcode op, int r, int a, char * c){
  TMInstruction i = {op, r, a-(emitLoc+1), pc, c};
  writeInstruction(emitLoc++, i);

  if (highEmitLoc < emitLoc) highEmitLoc = emitLoc;
} /* emitRM_Abs */

// Shortcuts (code by Jay Aslam)

void push(int reg, char *comm) {
  // ensure push starts with PUSH comment
  char* full_comment = calloc(strlen(comm)+7, sizeof(char));
  strcat(full_comment, "PUSH: ");
  strcat(full_comment, comm);

  // emit instructions
  emitRM(ST, reg, 0, sp, full_comment);
  emitRM(LDA, sp, -1, sp, "");

  free(full_comment);
}

void pop(int reg, char *comm) {
  // ensure push starts with PUSH comment
  char* full_comment = calloc(strlen(comm)+6, sizeof(char));
  strcat(full_comment, "POP: ");
  strcat(full_comment, comm);

  emitRM(LDA, sp, 1, sp, full_comment);
  emitRM(LD, reg, 0, sp, "");

  free(full_comment);
}

void emitOppositeSelStmt(int relop, int reg, int loc) {
  char* comment = "Generated by \"emitSelStmt\"";
  switch( relop ) {
  case relopLT:
    emitRMAbs(JGE, reg, loc, comment);
    break;
  case relopLE:
    emitRMAbs(JGT, reg, loc, comment);
    break;
  case relopGT:
    emitRMAbs(JLE, reg, loc, comment);
    break;
  case relopGE:
    emitRMAbs(JLT, reg, loc, comment);
    break;
  case relopEQ:
    emitRMAbs(JNE, reg, loc, comment);
    break;
  case relopNE:
    emitRMAbs(JEQ, reg, loc, comment);
    break;
  default:
    break;
  }
}

//////////////////////////////////////////
//  TM Instruction support functions
//////////////////////////////////////////

gboolean tmComparisonFunction(gconstpointer a, gconstpointer b);
void tmValueDestroyFunction(gpointer data);

// functions for the hash table implementation
gboolean tmComparisonFunction(gconstpointer a, gconstpointer b) {
  int* iA = (int*)a;
  int* iB = (int*)b;

  return (*iA) == (*iB);
}

void tmValueDestroyFunction(gpointer data) {
  TMInstruction* i = (TMInstruction*)data;

  // free the character strings and then the struct itself
  free(i->comment);
  free(i);
}

// initialized the codegen part
void codegenSupportInit() {
  instructionTable = g_hash_table_new_full( g_int_hash,
                                            tmComparisonFunction,
                                            free,
                                            tmValueDestroyFunction
                                          );
}

// writes an instruction to the hash table
void writeInstruction(int loc, TMInstruction i) {
  // first copy the instruction contents onto the heap
  TMInstruction* heapI = calloc(1, sizeof(TMInstruction));
  heapI->opCode = i.opCode;
  heapI->a = i.a;
  heapI->b = i.b;
  heapI->c = i.c;
  heapI->comment = calloc( strlen(i.comment)+1, sizeof(char) );
  strcpy(heapI->comment, i.comment);

  // add to the hash table
  int* key_ptr = malloc( sizeof(int) );
  *key_ptr = loc;
  g_hash_table_replace( instructionTable, key_ptr, heapI );
}

// performs a lookup on the instruction
TMInstruction* lookupInstruction(int label) {
  gpointer inst = g_hash_table_lookup(instructionTable, &label);

  return (TMInstruction*)inst;
}

// instruction types
// Type 0: OPC a,b(c)
// Type 1: OPC a,b,c
int getASMPrintType(Opcode op) {
  switch(op) {
    case HALT:
    case IN:
    case INF:
    case OUT:
    case OUTF:
    case ADD:
    case SUB:
    case MUL:
    case DIV:
    case NOP:
      return 1;
    default:
      return 0;
  }
}

// prints out the op code
void printOpcode(Opcode op) {
  switch(op) {
    case HALT:
      printf("%*s", 5, "HALT");
      break;
    case IN:
      printf("%*s", 5, "IN");
      break;
    case INF:
      printf("%*s", 5, "INF");
      break;
    case OUT:
      printf("%*s", 5, "OUT");
      break;
    case OUTF:
      printf("%*s", 5, "OUTF");
      break;
    case LD:
      printf("%*s", 5, "LD");
      break;
    case ST:
      printf("%*s", 5, "ST");
      break;
    case LDA:
      printf("%*s", 5, "LDA");
      break;
    case LDC:
      printf("%*s", 5, "LDC");
      break;
    case ADD:
      printf("%*s", 5, "ADD");
      break;
    case SUB:
      printf("%*s", 5, "SUB");
      break;
    case MUL:
      printf("%*s", 5, "MUL");
      break;
    case DIV:
      printf("%*s", 5, "DIV");
      break;
    case JEQ:
      printf("%*s", 5, "JEQ");
      break;
    case JNE:
      printf("%*s", 5, "JNE");
      break;
    case JLE:
      printf("%*s", 5, "JLE");
      break;
    case JGT:
      printf("%*s", 5, "JGT");
      break;
    case JGE:
      printf("%*s", 5, "JGE");
      break;
    case NOP:
      printf("%*s", 5, "NOP");
      break;
    default:
      printf("%*s", 5, "XXX");
  }
}

// prints out a single instruction
void printInstruction(int label, TMInstruction* i) {
  printf("%*d: ", 4, label);
  printOpcode(i->opCode);

  // if this is true print commas format,
  // otherwise use parenths
  if ( getASMPrintType(i->opCode) ) {
    printf("  %d,%d,%d\t\t%s\n", i->a, i->b, i->c, i->comment);
  } else {
    printf("  %d,%d(%d)\t\t%s\n", i->a, i->b, i->c, i->comment);
  }
}

// prints out the entire instruction table
void printInstructionTable(GHashTable* ht) {
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);
    printInstruction(n, inst);
  }
}
