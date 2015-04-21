#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "support.h"
#include "semantic_support.h"

// Headers for all functions in this file
static void program(TreeNode* nodePtr);
static void declarationList(TreeNode* nodePtr);
static void declaration(TreeNode* nodePtr);
static void varDeclaration(TreeNode* nodePtr);
static void funDeclaration(TreeNode* nodePtr);
static void params(TreeNode* nodePtr);
static void paramList(TreeNode* nodePtr);
static void param(TreeNode* nodePtr) ;
static void functionStmt(TreeNode* nodePtr);
static void compoundStmt(TreeNode* nodePtr);
static void localDeclaration(TreeNode* nodePtr);
static void statementList(TreeNode* nodePtr);
static void statement(TreeNode* nodePtr);
static void expressionStmt(TreeNode* nodePtr);
static void selectionStmt(TreeNode* nodePtr);
static void whileStmt(TreeNode* nodePtr);
static void forStmt(TreeNode* nodePtr);
static void jumpStmt(TreeNode* nodePtr);
static void returnStmt(TreeNode* nodePtr);
static void expression(TreeNode* nodePtr);
static void var(TreeNode* nodePtr, int rlval);
static void simpleExp(TreeNode* nodePtr);
static void additiveExp(TreeNode* nodePtr);
static void term(TreeNode* nodePtr);
static void factor(TreeNode* nodePtr);
static void call(TreeNode* nodePtr);
static void args(TreeNode* nodePtr);
static void argList(TreeNode* nodePtr);

// Evil (but necessary) Globals!
int         locals = 0;      // Number of local variables in the current scope
int         theNumParams;    // Number of params in the current function
int         in_while = 0;

int inlineable = 1;
int scopeDepth = 0;
int shouldInline = 1;

/****************************************************************************/
/*                                                                          */
/* semantic checking routines                                                 */
/*                                                                          */
/****************************************************************************/
void semanticCheck(TreeNode* nodePtr) {
    fprintf(stderr, "printing out tree...\n");
    printNode(nodePtr, NULL);

    program(nodePtr);
}

/* 1. prog -> declList */
static void program(TreeNode* nodePtr) {
    SemRec* semRecPtr;

    // Setup the top scope
    beginScope();                            // Push sym tab scope
    currentScope->base = gp;                    // Base = gp reg (4)
    currentScope->used = 0;                     // No spc used yet

    // Make a semantic rec for the input function
    semRecPtr = newSemRec();
    semRecPtr->f.kind = func;
    semRecPtr->f.numParams = 0;
    semRecPtr->f.localSpace = 0;
    semRecPtr->f.inlineable = 1;      // input function is inlineable by default
    insert(nodePtr->line, currentScope,"input",semRecPtr);

    // Make a semantic rec for the inputf function
    if ( with_float ) {
      semRecPtr = newSemRec();
      semRecPtr->f.kind = func;
      semRecPtr->f.numParams = 0;
      semRecPtr->f.localSpace = 0;
      semRecPtr->f.inlineable = 0;
      insert(nodePtr->line, currentScope,"inputf",semRecPtr);
    }

    // Make a semantic rec for the output function
    semRecPtr = newSemRec();
    semRecPtr->f.kind = func;
    semRecPtr->f.numParams = 1;
    semRecPtr->f.localSpace = 0;
    semRecPtr->f.inlineable = 1;      // output function is inlineable by default
    insert(nodePtr->line, currentScope,"output",semRecPtr);

    // Make a semantic rec for the outputf function
    if ( with_float ) {
      semRecPtr = newSemRec();
      semRecPtr->f.kind = func;
      semRecPtr->f.numParams = 1;
      semRecPtr->f.localSpace = 0;
      semRecPtr->f.inlineable = 0;
      insert(nodePtr->line, currentScope,"outputf",semRecPtr);
    }

    declarationList(nodePtr->ptr1);           // Check declList code

    // Clean up the top scope
    nodePtr->scope = currentScope;            // Save symbol table for later use
    currentScope = currentScope->prevScope;   // Pop scope
}

/* 2. declList -> decl declList | decl */
static void declarationList(TreeNode* nodePtr) {
    if (nodePtr->kind == declListNormal) {
    declaration(nodePtr->ptr1);           // Check declaration code
    declarationList(nodePtr->ptr2);       // Check declList code
    }
    else //if (nodePtr->kind == declListSingle)
    declaration(nodePtr->ptr1);           // Check declaration code
}

/* 3. decl -> varDecl | funDecl */
static void declaration(TreeNode* nodePtr) {
    if (nodePtr->kind == declVar)
    varDeclaration(nodePtr->ptr1);
    else //if (nodePtr->kind == declFun)
    funDeclaration(nodePtr->ptr1);
}

/* 4. varDecl -> typeSpec ID ';' | typeSpec ID '[' NUM ']' ';' */
/* 5. typeSpec -> INT | FLOAT | VOID */
static void varDeclaration(TreeNode* nodePtr) {
    SemRec* semRecPtr;

    // First case handles variables and
    // second case handles arrays
    if (nodePtr->kind == varDeclSingle) {
      // check to see if string is not NULL
      _test(nodePtr->ptr1,"vardecl->typespec");

      // Make sure it's not void
      if (nodePtr->ptr1->kind == typeSpecVoid) {
          fprintf(stderr, "Static semantic error!  Line %d, ",
              nodePtr->line);
          fprintf(stderr, "variable \"%s\" declared void.\n",
              nodePtr->value.string);
          exit(1);
      }

      // Make a semantic record for it, different for floats and ints
      semRecPtr = newSemRec();

      // set the appropriate kind
      if ( nodePtr->ptr1->kind == typeSpecInt ) {
        semRecPtr->v.kind = intvar;
      } else if ( nodePtr->ptr1->kind == typeSpecFloat ) {
        semRecPtr->v.kind = floatvar;
      }

      semRecPtr->v.base = currentScope->base;
      semRecPtr->v.offset = -currentScope->used;
      currentScope->used++;  // 1 more space has been used
      insert(nodePtr->line, currentScope, nodePtr->value.string, semRecPtr);

      // increment the locals counter
      locals++;
    }
    else { //if (nodePtr->kind == varDeclArray)
      _test(nodePtr->ptr1,"vardecl->typespec");

      // Make sure it's not void
      if (nodePtr->ptr1->kind == typeSpecVoid) { // VOID ID [NUM] is illegal
          fprintf(stderr, "Static semantic error!  Line %d, ",
              nodePtr->line);
          fprintf(stderr, "array \"%s\" declared void.\n",
              nodePtr->value.string);
          exit(1);
      }

      // Make sure it has length > 0
      if (nodePtr->array_len < 1) { // Num must be > 0
          fprintf(stderr, "Static semantic error!  Line %d, ",
              nodePtr->line);
          fprintf(stderr,"array \"%s\" has %d elements.\n",
              nodePtr->value.string, nodePtr->array_len);
          exit(1);
      }

      // Make a semantic record for it
      semRecPtr = newSemRec();

      // set the proper type depending on the typeSpec
      semRecPtr->v.kind = intarr;

      semRecPtr->v.base = currentScope->base;
      semRecPtr->v.offset = -currentScope->used;
      currentScope->used += nodePtr->array_len;
      insert(nodePtr->line, currentScope, nodePtr->value.string,semRecPtr);

      // increment the local variable counter by the
      // number of items in the array
      locals += nodePtr->array_len;
    }
}

/* 6. funDecl -> typeSpec ID '(' params ')' compStmt  */
static void funDeclaration(TreeNode* nodePtr) {
    // reset inlinable varaible and scope
    inlineable = 1;

     SemRec* semRecPtr;

     semRecPtr = newSemRec();
     semRecPtr->f.kind = func;
     semRecPtr->f.numParams = -1;            // Dummy value
     semRecPtr->f.localSpace = -1;           // Dummy value
     insert(nodePtr->line, currentScope, nodePtr->value.string,semRecPtr);

     beginScope();                           // Push scope onto sym tab
     currentScope->base = fp;                   // Base = FP
     currentScope->used = 2;                    // Spc for old fp, return addr
     theNumParams = 0;                       // No params yet

     params(nodePtr->ptr2);                  // Check params code

     semRecPtr->f.numParams = theNumParams;  // Record num of params
     locals = 0;                             // No spc for locals yet either

     functionStmt(nodePtr->ptr3);            // Check compStmt code

     semRecPtr->f.localSpace = locals;
     nodePtr->scope = currentScope;          // Save symbol table for later use
     currentScope = currentScope->prevScope; // Pop scope

     // set inlinable, don't inline main
     if ( strcmp(nodePtr->value.string, "main") == 0 ) {
       semRecPtr->f.inlineable = 0;
     } else {
       semRecPtr->f.inlineable = inlineable;
     }
}

/* 7. params -> paramList | VOID */
static void params(TreeNode* nodePtr) {
    if (nodePtr->kind == paramsNormal)
    paramList(nodePtr->ptr1);             // Check paramList code
    //else if (nodePtr->kind == paramsVoid)      // No checks needed.
}

/* 8. paramList -> param ',' paramList | param */
static void paramList(TreeNode* nodePtr) {
    if(nodePtr->kind == paramListNormal) {
    param(nodePtr->ptr1);                 // Check param code
    paramList(nodePtr->ptr2);             // Check paramList code
    } else //if (nodePtr->kind == paramListSingle)
    param(nodePtr->ptr1);                 // Ouput param code
}

/* 9. param -> typeSpec ID | typeSpec ID '[' ']' */
static void param(TreeNode* nodePtr) {
    SemRec* semRecPtr;

    if (nodePtr->kind == paramSingle) {

    // Make sure its not type void. 'void ID' is not legal.
    if (nodePtr->ptr1->kind == typeSpecVoid) {
        fprintf(stderr, "Static semantic error!  Line %d, ",
            nodePtr->line);
        fprintf(stderr, "variable parameter \"%s\" declared void.\n",
            nodePtr->value.string);
        exit(1);
    }

    // Make a semantic record for the parameter (local variable).
    semRecPtr = newSemRec();
    if ( nodePtr->ptr1->kind == typeSpecInt ) {
      semRecPtr->v.kind = intvar;
    } else if ( nodePtr->ptr1->kind == typeSpecFloat ) {
      semRecPtr->v.kind = floatvar;
    }
    semRecPtr->v.base = currentScope->base;
    semRecPtr->v.offset = -currentScope->used;
    currentScope->used++;
    insert(nodePtr->line, currentScope,nodePtr->value.string,semRecPtr);
    theNumParams++;
    }
    else { //if (nodePtr->kind ==paramArray)

    // Make sure its not type void. 'void ID []' is not legal.
    if (nodePtr->ptr1->kind == typeSpecVoid) {
        fprintf(stderr, "Semantic error!  Line %d, ", nodePtr->line);
        fprintf(stderr, "variable parameter \"%s\" declared void.\n",
            nodePtr->value.string);
        exit(1);
    }

    // Make a semantic record for the parameter (local array variable).
    semRecPtr = newSemRec();           // new semantic record (var)
    semRecPtr->v.kind = refarr;        // Kind = reference array
    semRecPtr->v.base = currentScope->base;    // set base loc
    semRecPtr->v.offset = -currentScope->used; // set offset loc
    currentScope->used++;                      // update spc used
    insert(nodePtr->line, currentScope, nodePtr->value.string,semRecPtr);
    theNumParams++;                         // inc num of params
    }
}

/* 10. funcStmt -> '{' localDecl stmtList '}' */
static void functionStmt(TreeNode* nodePtr) {
    scopeDepth = 0;

    localDeclaration(nodePtr->ptr1);          // Check localDecl code
    statementList(nodePtr->ptr2);             // Check stmtList code
}

/* 11. localDecl -> varDecl localDecl | empty */
static void localDeclaration(TreeNode* nodePtr) {
    if (nodePtr->kind == localDeclNormal) {
      varDeclaration(nodePtr->ptr1);        // Check  varDecl code
      localDeclaration(nodePtr->ptr2);      // Check localDecl code
    } //else if (nodePtr->kind == localDeclVoid) // No checks needed.
}

/* 12. stmtList -> stmt stmtList | empty */
static void statementList(TreeNode* nodePtr) {
    if (nodePtr->kind == stmtListNormal) {
      statement(nodePtr->ptr1);             // Check stmt code
      statementList(nodePtr->ptr2);         // Check stmtList code
    } //else if (nodePtr->kind == stmtListVoid)  // No checks needed.
}

/* 13. stmt -> expStmt | compStmt | selStmt | whileStmt | retStmt */
/* 13a. e_stmt -> expStmt | compStmt | e_selStmt | retStmt */
static void statement(TreeNode* nodePtr) {
    if (nodePtr->kind == stmtExp)
      expressionStmt(nodePtr->ptr1);        // Check expStmt code
    else if (nodePtr->kind == stmtComp)
      compoundStmt(nodePtr->ptr1);          // Check compStmt code
    else if (nodePtr->kind == stmtSel)
      selectionStmt(nodePtr->ptr1);         // Check selStmt code
    else if (nodePtr->kind == stmtWhile)
      whileStmt(nodePtr->ptr1);
    else if (nodePtr->kind == stmtFor)
      forStmt(nodePtr->ptr1);
    else if (nodePtr->kind == stmtJump)
      jumpStmt(nodePtr->ptr1);
    else //if (nodePtr->kind == stmtRet)
      returnStmt(nodePtr->ptr1);            // Check retStmt code
}

/* 14. expStmt -> exp ';' | ';' */
static void expressionStmt(TreeNode* nodePtr) {
    if (nodePtr->kind == expStmtNormal)
    expression(nodePtr->ptr1);            // Check code for exp
    //else if (nodePtr->kind == expStmtVoid)     // No checks needed.
}

/* 15. compStmt -> '{' localDecl stmtList '}' */
static void compoundStmt(TreeNode* nodePtr) {
    scopeDepth++;
    beginScope();                        // Push scope onto sym tab
    currentScope->base = fp;                // Base = FP
    currentScope->used = currentScope->prevScope->used;
                 // Same space amount as outer scope
    localDeclaration(nodePtr->ptr1);     // Check localDecl code
    statementList(nodePtr->ptr2);        // Check stmtList code
    nodePtr->scope = currentScope;      // Save symbol table for later use
    currentScope = currentScope->prevScope;    // Pop scope
}

/* 16. selStmt -> if '(' exp ')' stmt | if '(' exp ')' stmt else stmt */
/* 16a. e_selStmt -> if '(' exp ')' e_stmt else stmt */
static void selectionStmt(TreeNode* nodePtr) {
    if (nodePtr->kind == selStmtIf) {
      expression(nodePtr->ptr1);            // Check exp code
      statement(nodePtr->ptr2);             // Check stmt code
    } else { //if (nodePtr->kind == selStmtIfElse)
      expression(nodePtr->ptr1);            // Check exp code
      statement(nodePtr->ptr2);             // Check stmtExp code
      statement(nodePtr->ptr3);             // Check stmtComp code
    }
}

/* 17. whileStmt -> while '(' exp ')' stmt */
static void whileStmt(TreeNode* nodePtr) {
  // in while stmt
  int outermost_loop = !in_while;
  if ( outermost_loop ) {
    in_while = 1;
  }

  expression( nodePtr->ptr1 );
  statement( nodePtr->ptr2 );

  // reset the in_while statement
  if ( outermost_loop ) {
    in_while = 0;
  }
}

static void forStmt(TreeNode* nodePtr) {
  // treat forloops just like while loops
  int outermost_loop = !in_while;
  if ( outermost_loop ) {
    in_while = 1;
  }

  // process each of the expression pointers
  expression( nodePtr->ptr1 );
  expression( nodePtr->ptr2 );
  expression( nodePtr->ptr3 );

  // process the statements
  statement( nodePtr->ptr4 );

  // reset the in_while statement
  if ( outermost_loop ) {
    in_while = 0;
  }
}

/* 17a. jumpStmt -> BREAK | CONTINUE */
static void jumpStmt(TreeNode* nodePtr) {
  // make sure we are in a while loop
  if ( !in_while ) {
    fprintf(stderr, "Used either \"break\" or \"continue\" outside of while loop!\n");
    exit(-1);
  }
}

/* 18. retStmt -> return ';' | return exp ';' */
static void returnStmt(TreeNode* nodePtr) {
    // function is no longer inlineable if we are deeper than the top scope
    if ( scopeDepth > 0 ) {
      inlineable = 0;
    }

    //if (nodePtr->kind == retStmtVoid)          // No checks needed.
    if (nodePtr->kind == retStmtExp) {
      expression(nodePtr->ptr1);            // Check exp code
    }
}

/* 19. exp -> var '=' exp | simpExp */
static void expression(TreeNode* nodePtr) {
    if (nodePtr->kind == expAssign) {
    expression(nodePtr->ptr2);            // Check exp code
    var(nodePtr->ptr1, 0);                // Check var code
    } else //if (nodePtr->kind == expSimple)
    simpleExp(nodePtr->ptr1);             // Check simpleExp code
}

/* 20. var -> ID | ID '[' exp ']' */
static void var(TreeNode* nodePtr, int rlval) {
    SemRec* semRecPtr;

    // For variables...
    if (nodePtr->kind == varSingle) {
      semRecPtr = lookup(nodePtr->line, currentScope, nodePtr->value.string);

      // rlval == 0 checks the lvalue case
      // rlval == 1 checks the rvalue case
      if (rlval == 0) {

          // Make sure we don't take the l-value of something not an int or float.
          int is_not_single = !(semRecPtr->v.kind == intvar || semRecPtr->v.kind == floatvar);
          if (is_not_single) {
            fprintf(stderr, "Static semantic error!  Line %d, ",
                nodePtr->line);
            fprintf(stderr, "identifier \"%s\" not a legal l-value.\n",
                nodePtr->value.string);
            exit(1);
          }
      }
      else {// we want the rvalue
          // Make sure we don't take the r-value of a function
          if (semRecPtr->v.kind == func) {
            fprintf(stderr, "Static semantic error!  Line %d, ",
                nodePtr->line);
            fprintf(stderr, "identifier \"%s\" not a legal r-value.\n",
                nodePtr->value.string);
            exit(1);
          }
      }

    // For array variables...
    } else if (nodePtr->kind == varArray) {
      expression(nodePtr->ptr1);          // Check exp code

      semRecPtr = lookup(nodePtr->line, currentScope, nodePtr->value.string);

      // rlval == 0 checks the lvalue case
      // rlval == 1 checks the rvalue case
      if (rlval == 0){

          // Make sure we only take the l-value of an array.
          int not_array =
            semRecPtr->v.kind != intarr &&
            semRecPtr->v.kind != refarr;

          if (not_array) {
            fprintf(stderr, "Static semantic error!  Line %d, ",
                nodePtr->line);
            fprintf(stderr, "identifier \"%s\" not a legal l-value.\n",
                nodePtr->value.string);
            exit(1);
          }

      } else { // we want the rvalue

          // Make sure we only take the r-value of an array.
          int not_array =
            semRecPtr->v.kind != intarr &&
            semRecPtr->v.kind != refarr;

          if (not_array) {
            fprintf(stderr, "Static semantic error!  Line %d, ",
                nodePtr->line);
            fprintf(stderr, "identifier \"%s\" not a legal r-value.\n",
                nodePtr->value.string);
            exit(1);
          }
      }
    }
}

/* 21. simpExp -> addExp relop addExp | addExp */
/* 22. relop -> '<=' | '<' | '>' | '>=' | '==' | '!=' */
static void simpleExp(TreeNode* nodePtr) {
    if (nodePtr->kind == simpExpRelop) {
      additiveExp(nodePtr->ptr1);           // Check addExpNormal code
      additiveExp(nodePtr->ptr3);           // Check addExp2 code

      _test(nodePtr->ptr2, "simpexp->relop");
      if (nodePtr->ptr2->kind < relopLE || nodePtr->ptr2->kind > relopNE) {
          fprintf(stderr, "Static semantic error!  Line %d, ",
              nodePtr->line);
          fprintf(stderr, "unknown relational operator.\n");
      }

    } else //if (nodePtr->kind == simpExpAdditive)
      additiveExp(nodePtr->ptr1);           // Check addExp code
}

/* 23. addExp -> addExp addop term | term */
/* 24. addop -> '+' | '-' */
static void additiveExp(TreeNode* nodePtr) {
    if (nodePtr->kind == addExpNormal) {
      additiveExp(nodePtr->ptr1);           // Check addExp code
      term(nodePtr->ptr3);                  // Check term code
    } else //if (nodePtr->kind == addExp2)
      term(nodePtr->ptr1);                  // Check term code
}

/* 25. term -> term mulop factor | factor */
/* 26. mulop -> '*' | '/' */
static void term(TreeNode* nodePtr) {
    if (nodePtr->kind == termNormal) {
      term(nodePtr->ptr1);                  // Check term code
      factor(nodePtr->ptr3);                // Check factor code
    } else //if (nodePtr->kind == termFactor)
      factor(nodePtr->ptr1);                // Check factor code
}

/* 27. factor -> '(' exp ')' | var | call | NUM | FNUM */
static void factor(TreeNode* nodePtr) {
  switch( nodePtr->kind ) {
    case factorExp:
      expression(nodePtr->ptr1);            // Check exp code
      break;
    case factorVar:
      var(nodePtr->ptr1,1);                 // Check var code
      break;
    case factorCall:
      call(nodePtr->ptr1);                  // Check call code
      break;

    // don't do anything for NUM or FNUM
    default:
      break;
  }
}

/* 28. call -> ID '(' args ')' */
static void call(TreeNode* nodePtr) {
    SemRec* semRecPtr;

    semRecPtr = lookup(nodePtr->line, currentScope, nodePtr->value.string);

    args(nodePtr->ptr1);                      // Check args code

    if (semRecPtr->f.localSpace >= 0)
      nodePtr->locals_so_far = semRecPtr->f.localSpace;
    else
      nodePtr->locals_so_far = locals;
}

/* 29. args -> argList | empty */
static void args(TreeNode* nodePtr) {
    if (nodePtr->kind == argsNormal)
    argList(nodePtr->ptr1);               // Check argList code
    //else if (nodePtr->kind == argsVoid)        // No checks needed.
}

/* 30. argList -> exp ',' argList | exp */
static void argList(TreeNode* nodePtr) {
    if (nodePtr->kind == argListNormal) {
    expression(nodePtr->ptr1);            // Check exp code
    argList(nodePtr->ptr2);               // Check arglist code
    } else //if (nodePtr->kind == argListSingle)
    expression(nodePtr->ptr1);            // Check exp code
}

