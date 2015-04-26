#ifndef _TYPES_H
#define _TYPES_H

#include <glib.h>

// Types: int, array of int, array of ptr to int, function
typedef enum {intvar, intarr, floatvar, refarr, func} types_t;

// Kinds of parse tree nodes. One for each production (P1 : P2)
typedef enum {

  progNormal,
  declListNormal, declListSingle, declVar, declFun,
  varDeclSingle, varDeclArray,
  typeSpecInt, typeSpecFloat, typeSpecVoid, funDecl1, paramsNormal, paramsVoid, paramListNormal,
  paramListSingle, paramSingle, paramArray, compStmt1, localDeclNormal, localDeclVoid,
  stmtListNormal, stmtListVoid, funStmt1,
  stmtExp, stmtComp, stmtSel, stmtWhile, stmtFor, stmtJump, stmtRet,
  expStmtNormal, expStmtVoid,
  selStmtIf, selStmtIfElse,
  whileStmtNormal, forStmtNormal,
  jumpStmtBreak, jumpStmtContinue, retStmtVoid, retStmtExp, expAssign, expSimple, varSingle, varArray,
  simpExpRelop, simpExpAdditive, relopLE, relopLT, relopGT, relopGE, relopEQ,
  relopNE, addExpNormal, addExpTerm, addop, subop, termNormal, termFactor, mulopMult,
  mulopDiv, factorExp, factorVar, factorCall, factorNum, factorFloat,
  call1, callTail,
  argsNormal, argsVoid,
  argListNormal, argListSingle} nodekind_t;

/* *** structures for Semantic Checking ***********************************
 * type 'SemRec' (Semantic Record) contains the type and memory
 *        location of a variable or function.
 *
 * type 'HashNode' is a hash table node. Contains a pointer to a semRec.
 *
 * type 'HashTable' is a pointer to a node. An array of these
 *        HashTable foo[M] or HashTable* foo is the index to the hash table.
 *
 * type 'Scope' points to the HashTable for all the variables and functions
 *        in a given scope
 *
 * type 'SymbolTable' is a pointer to a Scope. The pointer to the current
 *        scope has this type.
 *
 * type 'TreeNode' is a parse tree node. It contains the name of the
 *        grammar rule that made it and a number or identifier name if it
 *        was NUM or ID. It also has pointer to any child parse tree nodes.
 * ************************************************************************/

/* Semantic Record node for a variable or a function.
 * The type SemRec is a union of two structures
 * Union means the SemRec is either a v (variable) or f (function)
 * It is accessed as if the union has two members v and f except that you may
 * only access one of them. Example: SemRec sr; sr.v.kind = intvar;
 * The type checker *should* notice if you try to use it as both v and f.
 */
typedef union {
  struct {
    types_t kind;     // Variable type (types that aren't func)
    int base;            // Memory location (usually fp - offset)
    int offset;
  } v;
  struct {
    types_t kind;            // Function type (always func)
    int addr;            // Memory location where the code is stored
    int numParams;       // Number of parameters
    int localSpace;      // Space for local variables
    int inlineable;      // holds whether or not variable can be inlined
    void* statements;    // holds the pointer to the start of the statements
    void* scope;         // scope for this function
  } f;
} SemRec;

// Symbol Table node. Points to a hash table of all the semantic
// records in its scope.
typedef struct snode {
  int           base;       // Frame Pointer
  int           used;       // Space used by variables
  GHashTable*   theTable;   // Pointer to this scope's semantic records
  struct snode* prevScope;  // Pointer to previous scope
} Scope;

// Parse Tree node.
typedef struct pnode {
    nodekind_t      kind;          // From the enum above (e.g. progNormal)

    union {
      int           integer;     // If it holds a number, its value
      float         floating;    // If it holds a float, its value
      char        * string;      // If it holds an id, its name
    } value;
    int             array_len;   // if it's an array, then here is its length

    struct pnode* ptr1;          // Pointers to any child nodes
    struct pnode* ptr2;
    struct pnode* ptr3;
    struct pnode* ptr4;
    int           line;          // Line number in the code

    int           locals_so_far; // used when making function calls
    Scope*   scope;        // used for nodes beginning a scope
} TreeNode,* ParseTree;

// opcodes
typedef enum {
  HALT, IN, INF, OUT, OUTF, LD, ST, LDA, LDC, LDF,
  ADD, SUB, MUL, DIV,
  JEQ, JNE, JLT, JLE, JGT, JGE,
  NOP
} Opcode;

// tiny machine constructions
typedef struct tminstruction {
  Opcode opCode;
  int a,b,c;
  char* comment;
} TMInstruction;

#endif

