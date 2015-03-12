#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "support.h"

// prints a node that is associated with its integer type
void printNodeType(TreeNode* node) {
  char* type;

  switch(node->kind) {
  case progNormal:
    type = "prg";
    break;
  case declListNormal:
  case declListSingle:
    type = "dcl";
    break;
  case declVar:
  case declFun:
    type = "dec";
    break;
  case varDeclSingle:
  case varDeclArray:
    type = "vdc";
    break;
  case typeSpecInt:
  case typeSpecFloat:
  case typeSpecVoid:
    type = "tsp";
    break;
  case funDecl1:
    type = "fdc";
    break;
  case paramsNormal:
  case paramsVoid:
    type = "prs";
    break;
  case paramListNormal:
  case paramListSingle:
    type = "prl";
    break;
  case paramSingle:
  case paramArray:
    type = "prm";
    break;
  case compStmt1:
    type = "cst";
    break;
  case localDeclNormal:
  case localDeclVoid:
    type = "ldc";
    break;
  case stmtListNormal:
  case stmtListVoid:
    type = "stl";
    break;
  case funStmt1:
    type = "fns";
    break;
  case stmtExp:
  case stmtComp:
  case stmtSel:
  case stmtWhile:
  case stmtRet:
  case stmtJump:
    type = "stm";
    break;
  case expStmtNormal:
  case expStmtVoid:
    type = "xst";
    break;
  case selStmtIf:
  case selStmtIfElse:
    type = "sst";
    break;
  case whileStmtNormal:
    type = "whi";
    break;
  case jumpStmtBreak:
    type = "jsb";
    break;
  case jumpStmtContinue:
    type = "jsc";
    break;
  case retStmtVoid:
  case retStmtExp:
    type = "rst";
    break;
  case expAssign:
  case expSimple:
    type = "exp";
    break;
  case varSingle:
  case varArray:
    type = "var";
    break;
  case simpExpRelop:
  case simpExpAdditive:
    type = "sxp";
    break;
  case relopLE:
  case relopLT:
  case relopGT:
  case relopGE:
  case relopEQ:
  case relopNE:
    type = "rop";
    break;
  case addExpNormal:
  case addExpTerm:
    type = "axp";
    break;
  case addop:
    type = "aop";
    break;
  case subop:
    type = "sop";
    break;
  case termNormal:
  case termFactor:
    type = "trm";
    break;
  case mulopMult:
  case mulopDiv:
    type = "mop";
    break;
  case factorExp:
  case factorVar:
  case factorCall:
  case factorNum:
  case factorFloat:
    type = "fct";
    break;
  case call1:
    type = "cal";
    break;
  case argsNormal:
  case argsVoid:
    type = "ags";
    break;
  case argListNormal:
  case argListSingle:
    type = "agl";
    break;
  }

  fprintf(stderr, "%s", type);
}

// recurses and prints out the contents of the folder
void printNode(TreeNode* node_ptr, GList* parent_stack) {
  // add to the parent stack
  parent_stack = g_list_append(parent_stack, node_ptr);

  // print out the right node
  if ( node_ptr->ptr1 != NULL ) {
    printNode( node_ptr->ptr1, parent_stack );
  }

  // figure out line placement
  int parent_length = g_list_length(parent_stack);
  if( parent_length > 1 ) {
    fprintf(stderr, "   ");
  }
  for( int n = 0; n < parent_length - 2; n++ ) {
    TreeNode* far_parent = (TreeNode*)g_list_nth( parent_stack, n )->data;
    TreeNode* mid_parent = (TreeNode*)g_list_nth( parent_stack, n+1 )->data;
    TreeNode* close_parent = (TreeNode*)g_list_nth( parent_stack, n+2 )->data;

    // print out line in special situation
    int right_then_left = (far_parent->ptr1 == mid_parent && mid_parent->ptr2 == close_parent);
    int left_then_right = (far_parent->ptr2 == mid_parent && mid_parent->ptr1 == close_parent);
    if( right_then_left || left_then_right ) {
      fprintf(stderr, "|   ");
    } else {
      fprintf(stderr, "    ");
    }
  }

  // print the * right before the value of the node
  int depth = parent_length - 1;

  // fix special case of depth=1
  if ( depth != 0 ) {
    fprintf(stderr, "*");
  }

  // print out the node value
  printNodeType(node_ptr);

  if ( node_ptr->ptr1 != NULL || node_ptr->ptr2 != NULL ) {
    fprintf(stderr, "+");
  }

  // end the line
  fprintf(stderr, "\n");

  // print out the left node
  if ( node_ptr->ptr2 != NULL ) {
    printNode( node_ptr->ptr2, parent_stack );
  }

  // take off this node from the parent stack
  parent_stack = g_list_remove( parent_stack, node_ptr );
}

/****************************************************************************/
/*                                                                          */
/* code generation routines                                                 */
/*                                                                          */
/****************************************************************************/

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

/****************************************************************************/
/*                                                                          */
/* symbol table                                                             */
/*                                                                          */
/****************************************************************************/

SemRec* lookup(int line, SymbolTable scopePtr, char* theName) {
     int slot;
     SemRec* tempRec;
     HashNode* tempNode;

     slot = hashFunct(theName);      // Hash the key.

     // If the scope is NULL, the SemRec does not exist, error.
     if (scopePtr == NULL) {
	  fprintf(stderr, "Variable/Function does not exist.\n");
	  fprintf(stderr, "Line = %d, Name = %s \n", line, theName);
	  exit(1);
     }

     tempNode = scopePtr->theTable[slot];  // Look for the node.
     while((tempNode != NULL) && strcmp(tempNode->key,theName))
	  tempNode = tempNode->nextNode;

     if (tempNode == NULL)                 // If not found, look in next scope.
	  tempRec = lookup(line, scopePtr->prevScope, theName);
     else                                  // Else, retrieve it.
	  tempRec = tempNode->theSemRec;

     return tempRec;
}

void insert(int line, SymbolTable scopePtr, char* theName, SemRec* theRec) {
     int slot = hashFunct(theName);            // Hash the key.

     HashNode* box = newHTableNode();              // Make a node for it.

     // copy the string
     box->key = calloc(strlen(theName), sizeof(char));
     strcpy(box->key, theName);

       box->theLine = line;
     box->theSemRec = theRec;
     box->nextNode = NULL;

     box->nextNode = scopePtr->theTable[slot]; // Insert the node.
     scopePtr->theTable[slot] = box;
}

int hashFunct(char* theName) {
     int len,i, value;
     value = 0;

     len = strlen(theName);
     for (i=0;i<len;i++)
	  value += theName[i];

     return (value % M);
}

// Make a new scope and push it on the symbol table
void beginScope() {
    Scope* tempScope = newScope();              // Make a new scope
    tempScope->theTable = newHashTable();        // Make a hash table
    tempScope->prevScope = symTabPtr;               // Set the prev scope
    symTabPtr = tempScope;                          // Point to the scope
}

// Remove a scope
void endScope() {
    int i;
    HashNode* tempNodeP, * tempNodeP2;
    Scope* tempScopeP = symTabPtr;                 // Point to scope
    symTabPtr = tempScopeP->prevScope;              // Scope is prev scope

    for (i=0; i<M; i++) {                        // Destroy old hash table
	tempNodeP = tempScopeP->theTable[i];
	while (tempNodeP != NULL) {
	    free(tempNodeP->theSemRec);
	    free(tempNodeP->key);
	    tempNodeP2 = tempNodeP->nextNode;
	    free(tempNodeP);
	    tempNodeP = tempNodeP2;
	}
    }
    free(tempScopeP->theTable);

    free(tempScopeP);                            // Destroy old scope
}


/****************************************************************************/
/*                                                                          */
/* utilities                                                                */
/*                                                                          */
/****************************************************************************/


char* myalloc(size_t size) {
     char* temp;
     if ((temp = malloc(size)) != NULL)
	  return temp;
     else {
	  fprintf(stderr, "Out of memory!\n");
	  fprintf(stderr, "(Successfully compiled %d lines.)\n", lineno);
	  exit(1);
     }
}

TreeNode* newParseTreeNode() {
     TreeNode* temp;
     temp = (TreeNode*) myalloc(sizeof(TreeNode));
     temp->line = lineno;
     return temp;
}

Scope* newScope() { return (Scope*) myalloc(sizeof(Scope)); }
SemRec* newSemRec() { return (SemRec*) myalloc(sizeof(SemRec)); }
HashNode* newHTableNode() { return (HashNode*) myalloc(sizeof(HashNode)); }

HashTable* newHashTable() {
     int i;
     HashTable* temp = (HashTable*) myalloc(M*sizeof(HashTable));
     for(i = 0; i<M; i++) temp[i]=NULL;
     return temp;
}

//////////////////////////////////////////
//  TM Instruction support functions
//////////////////////////////////////////
GHashTable* instructionTable;

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

/*
 * Code Profiler
 */
void _nopifyInstruction(int loc) {
  TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &loc);
  inst->opCode = NOP;
  inst->a = 0;
  inst->b = 0;
  inst->c = 0;
}

void profileLDST() {
  int state = 0;
  int numIdentified = 0;
  int a,b,c;
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    if ( inst->opCode == LD ) {
      a = inst->a;
      b = inst->b;
      c = inst->c;

      state = 1;
    } else if ( inst->opCode == ST &&
                inst->a == a &&
                inst->b == b &&
                inst->c == c ) {
      if ( state == 1 ) {
        // nopify both instructions
        _nopifyInstruction(n-1);
        _nopifyInstruction(n);
        numIdentified++;
      }

      state = 0;
    } else {
      state = 0;
    }
  }

  fprintf(stderr, "LDST: %i\n", numIdentified);
}

void profileSTLD() {
  int state = 0;
  int numIdentified = 0;
  int a,b,c;
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    if ( inst->opCode == ST ) {
      a = inst->a;
      b = inst->b;
      c = inst->c;

      state = 1;
    } else if ( inst->opCode == LD &&
                inst->a == a &&
                inst->b == b &&
                inst->c == c ) {
      if ( state == 1 ) {
        // nopify both instructions
        _nopifyInstruction(n-1);
        _nopifyInstruction(n);
        numIdentified++;
      }

      state = 0;
    } else {
      state = 0;
    }
  }

  fprintf(stderr, "STLD: %i\n", numIdentified);
}

void profileSTST() {
  int state = 0;
  int numIdentified = 0;
  int b,c;
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    if ( inst->opCode == ST ) {
      b = inst->b;
      c = inst->c;

      state = 1;
    } else if ( inst->opCode == ST &&
                inst->b == b &&
                inst->c == c ) {
      if ( state == 1 ) {
        // nopify the second op
        _nopifyInstruction(n);
        numIdentified++;
      }

      state = 0;
    } else {
      state = 0;
    }
  }

  fprintf(stderr, "STST: %i\n", numIdentified);
}

void profileLDLD() {
  int state = 0;
  int numIdentified = 0;
  int b,c;
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    if ( inst->opCode == LD ) {
      b = inst->b;
      c = inst->c;

      state = 1;
    } else if ( inst->opCode == LD &&
                inst->b == b &&
                inst->c == c ) {
      if ( state == 1 ) {
        // nopify the second op
        _nopifyInstruction(n);
        numIdentified++;
      }

      state = 0;
    } else {
      state = 0;
    }
  }

  fprintf(stderr, "LDLD: %i\n", numIdentified);
}

void profileADDSUB() {
  int state = 0;
  int numIdentified = 0;
  int X,Y;
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    if ( inst->opCode == ADD &&
         inst->a == inst->b ) {
      X = inst->a;
      Y = inst->c;

      state = 1;
    } else if ( inst->opCode == SUB &&
                inst->a == X &&
                inst->b == X &&
                inst->c == Y ) {
      if ( state == 1 ) {
        // nopify the instructions
        _nopifyInstruction(n-1);
        _nopifyInstruction(n);
        numIdentified++;
      }

      state = 0;
    } else {
      state = 0;
    }
  }

  fprintf(stderr, "ADDSUB: %i\n", numIdentified);
}

void profileSUBADD() {
  int state = 0;
  int numIdentified = 0;
  int X,Y;
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    if ( inst->opCode == SUB &&
         inst->a == inst->b ) {
      X = inst->a;
      Y = inst->c;

      state = 1;
    } else if ( inst->opCode == ADD &&
                inst->a == X &&
                inst->b == X &&
                inst->c == Y ) {
      if ( state == 1 ) {
        // nopify the instructions
        _nopifyInstruction(n-1);
        _nopifyInstruction(n);
        numIdentified++;
      }

      state = 0;
    } else {
      state = 0;
    }
  }

  fprintf(stderr, "SUBADD: %i\n", numIdentified);
}

void profileMULDIV() {
  int state = 0;
  int numIdentified = 0;
  int X,Y;
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    if ( inst->opCode == MUL &&
         inst->a == inst->b ) {
      X = inst->a;
      Y = inst->c;

      state = 1;
    } else if ( inst->opCode == DIV &&
                inst->a == X &&
                inst->b == X &&
                inst->c == Y ) {
      if ( state == 1 ) {
        // nopify the instructions
        _nopifyInstruction(n-1);
        _nopifyInstruction(n);
        numIdentified++;
      }

      state = 0;
    } else {
      state = 0;
    }
  }

  fprintf(stderr, "MULDIV: %i\n", numIdentified);
}

void profileDIVMUL() {
  int state = 0;
  int numIdentified = 0;
  int X,Y;
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    if ( inst->opCode == DIV &&
         inst->a == inst->b ) {
      X = inst->a;
      Y = inst->c;

      state = 1;
    } else if ( inst->opCode == MUL &&
                inst->a == X &&
                inst->b == X &&
                inst->c == Y ) {
      if ( state == 1 ) {
        // nopify the instructions
        _nopifyInstruction(n-1);
        _nopifyInstruction(n);
        numIdentified++;
      }

      state = 0;
    } else {
      state = 0;
    }
  }

  fprintf(stderr, "DIVMUL: %i\n", numIdentified);
}

// reutrns the number of unnecessary instructions
void _nopifyPushPop(int pushStart, int popStart) {
  // nopify the PUSH instructions
  _nopifyInstruction(pushStart);
  _nopifyInstruction(pushStart + 1);

  // nopify the POP instructions
  _nopifyInstruction(popStart);
  _nopifyInstruction(popStart + 1);
}

int _pushpopMatch(int start, int X) {
  // store the number of redundant push/pops, whether or not this
  // push/pop pair should count, and how many push/pops we are deep
  int isValid = 1;
  int depth = 1;

  // iterate instructions either until the end or until we have broken out of
  // the push/pop pair
  int i;
  for ( i = start; ( i < g_hash_table_size(instructionTable) ) && (depth > 0); i++ ) {
    // get the next instruction
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &i);

    // if we see another PUSH statement, then increment the depth counter
    int compare = strncmp("PUSH: ", inst->comment, 6);
    if ( compare == 0 ) {
      depth++;
      continue;
    }

    // break out if we find a POP, successful identification
    compare = strncmp("POP: ", inst->comment, 5);
    if ( compare == 0 ) {
      depth--;
      continue;
    }

    // make sure we don't look into another function call
    compare = strncmp("CALL:", inst->comment, 5);
    if ( compare == 0 ) {
      isValid = 0;
    }

    // if there was an assignment, the current push/pop sequence doesn't count
    // keep iterating until the appropro pop statement
    if ( inst->a == X ) {
      switch ( inst->opCode ) {
      case ADD:
      case SUB:
      case MUL:
      case DIV:
      case LD:
      case LDA:
      case LDC:
      case IN:
        isValid = 0;
        break;
      default:
        break;
      }
    }
  }

  // only perform swap where it counts, if isValid == 0, that means
  // there was an instruction that overwrote that register
  if ( isValid ) {
    _nopifyPushPop(start-2, i);
  }

  return isValid;
}

// profiles the PUSH and POP operations, checks to see if we are pushing
// something that never gets assigned to before we pop.
void profilePUSHPOP() {
  int numIdentified = 0;

  // run this part from the beginning
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    // get the next instruction
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    // recurse again if we found another PUSH instruction
    int compare = strncmp("PUSH: ", inst->comment, 6);
    if ( compare == 0 ) {
      numIdentified += _pushpopMatch(n+2, inst->a);
    }

  }

  fprintf(stderr, "PUSHPOP: %i\n", numIdentified);
}

int _pushToCopyMatch( int start, int X ) {
  int depth = 1;      // keep track of push/pop depth
  int isValid = 1;    // assume is valid until another write operation
  int i;              // keep track of position
  for ( i = start; (i > -1) && (depth > 0); i-- ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &i);

    // look for matching PUSH instructions
    int compare = strncmp("PUSH: ", inst->comment, 6);
    if ( compare == 0 ) {
      depth--;
      continue;
    }

    compare = strncmp("POP: ", inst->comment, 5);
    if ( compare == 0 ) {
      depth++;
      continue;
    }

    // make sure we don't look into another function
    compare = strncmp("CALL:", inst->comment, 5);
    if ( compare == 0 ) {
      isValid = 0;
    }

    // don't count this one if we assign to the register we pop at
    if ( inst->a == X ) {
      switch ( inst->opCode ) {
      case ADD:
      case SUB:
      case MUL:
      case DIV:
      case LD:
      case LDA:
      case LDC:
      case IN:
        isValid = 0;
        break;
      default:
        break;
      }
    }
  }

  // if there was no assignment to POP register over whole time,
  // then we should perform optimization
  if ( isValid && i > -1 ) {
    // get the register to save
    int loc = i+1;
    TMInstruction* ti = (TMInstruction*)g_hash_table_lookup(instructionTable, &loc);
    int regToSave = ti->a;

    // nopify all the instructions
    _nopifyPushPop(loc, start+1);

    ti->opCode = LDA;
    ti->a = X;
    ti->b = 0;
    ti->c = regToSave;
  }

  return isValid;
}
void profilePUSHTOCOPY() {
  int numIdentified = 0;

  // start from the end since we want outside instructions to change
  // before inside instructions
  for ( int n = g_hash_table_size(instructionTable)-1; n > 0; n-- ) {
    // get the next instruction
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    int compare = strncmp("POP: ", inst->comment, 5);
    if ( compare == 0 ) {
      // get the next instruction to find the target register
      int loc = n+1;
      TMInstruction* ti = (TMInstruction*)g_hash_table_lookup(instructionTable, &loc);
      numIdentified += _pushToCopyMatch(n-1, ti->a);
    }
  }

  fprintf(stderr, "PUSHTOCOPY: %i\n", numIdentified);
}

void profile() {
  fprintf(stderr, "Profile Results ======\n");

  profileLDST();
  profileSTLD();
  profileSTST();
  profileLDLD();
  profileADDSUB();
  profileSUBADD();
  profileMULDIV();
  profileDIVMUL();
  profilePUSHPOP();
  profilePUSHTOCOPY();

  fprintf(stderr, "======================\n");
}
