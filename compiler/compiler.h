#ifndef _COMPILER_H
#define _COMPILER_H

#define pc  7            // Program Counter (next instruction addr)
#define sp  6            // Stack Pointer (top of stack)
#define fp  5            // Frame Pointer (top of current frame in stack)
#define gp  4            // Globals Pointer (top of program)
#define ac1 1            // Accumulator 1
#define ac0 0            // Accumulator 0 (return values go here)

#define YYSTYPE ParseTree    // The parser forms a parse tree

#define M 11

// Types: int, array of int, array of ptr to int, function
typedef enum {intvar, intarr, refarr, func} types_t;

// Kinds of parse tree nodes. One for each production (P1 : P2)
typedef enum {progNormal, declListNormal, declListSingle, declVar, declFun, varDeclSingle, varDeclArray,
      typeSpecInt, typeSpecFloat, typeSpecVoid, funDecl1, paramsNormal, paramsVoid, paramListNormal,
      paramListSingle, paramSingle, paramArray, compStmt1, localDeclNormal, localDeclVoid,
      stmtListNormal, stmtListVoid, funStmt1,
      stmtExp, stmtComp, stmtSel, stmt4, stmtRet, expStmtNormal, expStmtVoid, selStmtIf,
      selStmtIfElse, iterStmt1, retStmtVoid, retStmtExp, expAssign, expSimple, varSingle, varArray,
      simpExp1, simpExp2, relopLE, relopLT, relopGT, relopGE, relopEQ,
      relopNE, addExpNormal, addExpTerm, addop, subop, termNormal, termFactor, mulopMult,
      mulopDiv, factorExp, factorVar, factorCall, factorNum, call1, argsNormal, argsVoid,
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
    } f;
} SemRec;

// Hash Table node. Points to a semantic record.
typedef struct hnode {
    char*         key;             // Variable or function name
    int           theLine;          // Code line in .cm file
    SemRec*       theSemRec;       // The semantic record.
    struct hnode* nextNode;
} HashNode,* HashTable;

// Symbol Table node. Points to a hash table of all the semantic
// records in its scope.
typedef struct snode {
    int           base;       // Frame Pointer
    int           used;       // Space used by variables
    HashTable*    theTable;  // Pointer to this scope's semantic records
    struct snode* prevScope; // Pointer to previous scope
} Scope,* SymbolTable;

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
    int           line;          // Line number in the code

    int           locals_so_far; // used when making function calls
    SymbolTable   symTabPtr;        // used for nodes beginning a scope
} TreeNode,* ParseTree;

extern SymbolTable   symTabPtr;
extern ParseTree     parseTreePtr;
extern int           lineno;

// Semantic checking support functions
extern SemRec*    lookup(int line, SymbolTable scopePtr, char* theName);
extern void       insert(int line, SymbolTable scopePtr, char* theName,
			 SemRec* theRec);
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

// Code Generation support functions
extern int  emitSkip(int howMany);
extern void emitBackup(int loc);
extern void emitRestore(void);
extern void emitComment(char*  c);
extern void emitRO(char* op, int r, int s, int t, char* c);
extern void emitRM(char* op, int r, int d, int s, char* c);
extern void emitRMAbs( char* op, int r, int a, char*  c);
extern void push(int reg, char* comm) ;
extern void pop(int reg, char* comm);

// For debugging purposes
#define _test(a,b)  if (NULL==a) fprintf(stderr, "ACCESSING NULL %s\n", b);

#endif
