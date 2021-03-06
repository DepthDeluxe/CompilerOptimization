#include <stdio.h> // fprintf(), stderr
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "types.h"
#include "compiler.h"

#include "codegen_support.h"
#include "semantic_support.h"

GHashTable* instructionTable;
Scope*      currentScope;

// Headers for all functions in this file.
// static means the function cannot be used outside of this file
// therefore it's ok for semantic.c and codegen.c to use the
// same function names.
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
static void expression(TreeNode* nodePtr, int noJump);
static void var(TreeNode* nodePtr, int rlval);
static void simpleExp(TreeNode* nodePtr, int noJump);
static void additiveExp(TreeNode* nodePtr);
static void term(TreeNode* nodePtr);
static void factor(TreeNode* nodePtr);
static void call(TreeNode* nodePtr);
static void args(TreeNode* nodePtr);
static void argList(TreeNode* nodePtr);

/* evil globals, really shouldn't be using theses but IDGAF */
static int whileConditionalLocation = -1;
static int whileJumpLocation = -1;
static nodekind_t whileConditionalType;

static char functionName[50];
static char variableName[50];
static int inNonInlineableExp = 0;
static int insideInline = 0;
static int numInlineCalls = 0;

extern int with_inlining;


/****************************************************************************/
/*                                                                          */
/* code generation routines                                                 */
/*                                                                          */
/****************************************************************************/
void generateCode(TreeNode* nodePtr) {
  program(nodePtr);

  if ( with_inlining ) {
    fprintf(stderr, "Number inline calls: %i\n", numInlineCalls);
  }
}

/* 1. prog -> declList */
static void program(TreeNode* nodePtr) {
    SemRec* semRecPtr;
    int loc, main1, main2;

    currentScope = nodePtr->scope;       // Get global symbol table

    // Run-time environment setup instructions
    emitRM(LD,gp,0,ac0,"Set GP");                // GP = dMem[0] = size-1
    loc = emitSkip(2);                             // Init FP,SP here later
    emitRM(ST,ac0,0,ac0,"Clear dMem[0]");        // re-init dMem[0] = 0
    push(fp,"Save FP");                            // dMem[sp] = fp
    main1 = emitSkip(1);                           // Re-Init SP here later
    emitRM(LDA,ac0,2,pc,"Get PC for Halt");      // ac0 has PC for HALT line
    emitRM(ST,ac0,-1,fp,"Function: store return address in dMem[FP-1]");
    main2 = emitSkip(1);                           // Init PC here
    emitRO(HALT,0,0,0,"Halt");                   // Stop.

    // provide input function
    semRecPtr = lookup(nodePtr->line, currentScope, "input");
    semRecPtr->f.addr = emitSkip(0);
    emitRM(ST,ac0,-1,fp,"Input Function, store return addr");
    emitRO(IN,ac0,0,0,  "     Get input");
    emitRM(LD,pc,-1,fp, "     Return (end of function)");

    // provide inputf function
    if ( with_float ) {
      semRecPtr = lookup(nodePtr->line, currentScope, "input");
      semRecPtr->f.addr = emitSkip(0);
      emitRM(ST,ac0,-1,fp,"Inputf Function, store return addr");
      emitRO(INF,ac0,0,0,  "     Get input");
      emitRM(LD,pc,-1,fp, "     Return (end of function)");
    }

    // provide output function
    semRecPtr = lookup(nodePtr->line, currentScope, "output");
    semRecPtr->f.addr = emitSkip(0);
    emitRM(ST,ac0,-1,fp,"Output Function, store return addr");
    emitRM(LD,ac0,-2,fp,"     Get output");
    emitRO(OUT,ac0,0,0, "     Give output");
    emitRM(LD,pc,-1,fp, "     Return (end of function)");

    // provide outputf function
    if ( with_float ) {
      semRecPtr = lookup(nodePtr->line, currentScope, "outputf");
      semRecPtr->f.addr = emitSkip(0);
      emitRM(ST,ac0,-1,fp,"Outputf Function, store return addr");
      emitRM(LD,ac0,-2,fp,"     Get output");
      emitRO(OUTF,ac0,0,0, "     Give output");
      emitRM(LD,pc,-1,fp, "     Return (end of function)");
    }

    declarationList(nodePtr->ptr1);                  // Code for declList

    emitBackup(loc);
    emitRM(LDA,fp,-currentScope->used,gp,"Set FP below globals");
    emitRM(LDA,sp,-currentScope->used,gp,"Set SP below globals");
    emitRestore();
    semRecPtr = lookup(nodePtr->line, currentScope, "main");
    emitBackup(main2);
    emitRMAbs(LDA,pc,semRecPtr->f.addr,"Jump to main");// PC = main
    emitRestore();
    emitBackup(main1);
    emitRM(LDA,sp,-(semRecPtr->f.localSpace+1),sp,"Set SP below locals");
    emitRestore();
    endScope();
}

/* 2. declList -> decl declList | decl */
static void declarationList(TreeNode* nodePtr) {
    if (nodePtr->kind == declListNormal) {
      declaration(nodePtr->ptr1);
      declarationList(nodePtr->ptr2);
    }
    else //if (nodePtr->kind == declListSingle) {
      declaration(nodePtr->ptr1);
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
    // Variable declaration - No code to generate
}

/* 6. funDecl -> typeSpec ID '(' params ')' compStmt  */
static void funDeclaration(TreeNode* nodePtr) {
    SemRec* semRecPtr;

    strcpy(functionName, nodePtr->value.string);

    semRecPtr = lookup(nodePtr->line, currentScope, nodePtr->value.string);
    semRecPtr->f.addr = emitSkip(0);           // Func code starts here
    semRecPtr->f.statements = nodePtr->ptr3;  // emit the statments
    semRecPtr->f.scope = nodePtr->scope;      // save the scope for later (like everything else)
    currentScope = nodePtr->scope;                  // Get current symbol table

    char* comment = calloc(500, sizeof(char));
    sprintf(comment, "Start of function: %s", nodePtr->value.string);
    emitRM(LDA, ac0, 0, ac0, comment);
    free(comment);

    params(nodePtr->ptr2);                     // Output params code
    //emitRM(ST,ac0,-1,fp,"Function: store return address in dMem[FP-1]");
    functionStmt(nodePtr->ptr3);               // Output compStmt code
    emitRM(LD,pc,-1,fp,"End of function (set PC = return addr)");
    endScope();                                // Pop scope
}

/* 7. params -> paramList | VOID */
static void params(TreeNode* nodePtr) {
    if (nodePtr->kind == paramsNormal)
      paramList(nodePtr->ptr1);                  // Output paramList code
    //else if (nodePtr->kind == paramsVoid) // No code to generate
}

/* 8. paramList -> param ',' paramList | param */
static void paramList(TreeNode* nodePtr) {
    if(nodePtr->kind == paramListNormal) {
      param(nodePtr->ptr1);                      // Output param code
      paramList(nodePtr->ptr2);                  // Output paramList code
    } else //if (nodePtr->kind == paramListSingle) {
      param(nodePtr->ptr1);                      // Ouput code for param
}

/* 9. param -> typeSpec ID | typeSpec ID '[' ']' */
static void param(TreeNode* nodePtr) {
    // Parameter declaration - No code to generate
}

/* 10. funcStmt -> '{' localDecl stmtList '}' */
static void functionStmt(TreeNode* nodePtr) {

    localDeclaration(nodePtr->ptr1);          // Output code for localDecl
    statementList(nodePtr->ptr2);             // Output code for stmtList
}

/* 11. localDecl -> varDecl localDecl | empty */
static void localDeclaration(TreeNode* nodePtr) {
    if (nodePtr->kind == localDeclNormal) {
      varDeclaration(nodePtr->ptr1);         // Output code for varDecl
      localDeclaration(nodePtr->ptr2);       // Output code for localDecl
    }
}

/* 12. stmtList -> stmt stmtList | empty */
static void statementList(TreeNode* nodePtr) {
    if (nodePtr->kind == stmtListNormal) {
      statement(nodePtr->ptr1);              // Output code for stmt
      statementList(nodePtr->ptr2);          // Output code for stmtList
    }
}

/* 13. stmt -> expStmt | compStmt | selStmt | retStmt */
/* 13a. e_stmt -> expStmt | compStmt | e_selStmt | retStmt */
static void statement(TreeNode* nodePtr) {
    if (nodePtr->kind == stmtExp)
      expressionStmt(nodePtr->ptr1);   // Output code for expStmt
    else if (nodePtr->kind == stmtComp)
      compoundStmt(nodePtr->ptr1);     // Output code for compStmt
    else if (nodePtr->kind == stmtSel)
      selectionStmt(nodePtr->ptr1);    // Output code for selStmt
    else if (nodePtr->kind == stmtWhile)
      whileStmt( nodePtr->ptr1 );
    else if (nodePtr->kind == stmtFor)
      forStmt( nodePtr->ptr1 );
    else if (nodePtr->kind == stmtJump)
      jumpStmt( nodePtr-> ptr1 );
    else //if (nodePtr->kind == stmtRet)
      returnStmt(nodePtr->ptr1);       // Output code for retStmt
}

/* 14. expStmt -> exp ';' | ';' */
static void expressionStmt(TreeNode* nodePtr) {
    if (nodePtr->kind == expStmtNormal)
      expression(nodePtr->ptr1, 0);        // Output code for exp
    //else if (nodePtr->kind == expStmtVoid) // No code to generate
}

/* 15. compStmt -> '{' localDecl stmtList '}' */
static void compoundStmt(TreeNode* nodePtr) {
    currentScope = nodePtr->scope;           // Get current symbol table
    localDeclaration(nodePtr->ptr1);    // Output code for localDecl
    statementList(nodePtr->ptr2);       // Output code for stmtList
    endScope();                         // Pop scope
}

/* 16. selStmt -> if '(' exp ')' stmt | if '(' exp ')' stmt else stmt */
/* 16a. e_selStmt -> if '(' exp ')' e_stmt else stmt */
static void selectionStmt(TreeNode* nodePtr) {
    int startLoc, elseLoc, endIfLoc, endLoc;
    if (nodePtr->kind == selStmtIf) {
      inNonInlineableExp = 1;
      expression(nodePtr->ptr1, 1);     // Output exp code
      inNonInlineableExp = 0;

      startLoc = emitSkip(1);             // Save space for "if" test
      statement(nodePtr->ptr2);      // Output stmt code

      endLoc = emitSkip(0);            // Get jump destination (end)
      emitBackup(startLoc);               // Backup numbering to "if" test
      TreeNode* expStmtNode = nodePtr->ptr1;
      TreeNode* simpExpNode = expStmtNode->ptr1;
      if ( simpExpNode->kind == simpExpRelop ) {
        emitOppositeSelStmt(simpExpNode->ptr2->kind, ac0, endLoc);
      } else {
        emitRMAbs(JEQ,ac0,endLoc,"  tests");     // Jump to end
      }
      emitRestore();                 // Restore numbering
    } else { //if (nodePtr->kind == selStmtIfElse)

      inNonInlineableExp = 1;
      expression(nodePtr->ptr1, 1);        // Output exp code
      inNonInlineableExp = 0;

      startLoc = emitSkip(1);                // Skip jump1 loc
      statement(nodePtr->ptr2);         // Output stmtExp code

      emitRM(LDC,ac0,0,0,
             "  if: Put a 0 in ac0 so we jump over the else part");
      endIfLoc = emitSkip(1);               // Skip jump2 loc
      elseLoc = emitSkip(0);               // Get jump1 destination (else)
      statement(nodePtr->ptr3);         // Output stmtComp code
      endLoc = emitSkip(0);               // Get jump2 destination (end)

      emitBackup(startLoc);                  // Go back to jump1 loc
      TreeNode* expStmtNode = nodePtr->ptr1;
      TreeNode* simpExpNode = expStmtNode->ptr1;
      if ( simpExpNode->kind == simpExpRelop ) {
        emitOppositeSelStmt(simpExpNode->ptr2->kind, ac0, elseLoc);
      } else {
        emitRMAbs(JEQ,ac0, elseLoc,"  testts");     // Jump to end
      }
      emitRestore();                    // Restore instr counter to else

      emitBackup(endIfLoc);                 // Go back to jump2 loc
      emitRMAbs(JEQ,ac0,endLoc,"  if: Jump to the end");     // Jump to end
      emitRestore();                    // Restore instr counter to end
    }
}

/* 17. whileStmt -> while '(' exp ')' stmt */
static void whileStmt( TreeNode* nodePtr ) {
  // save away the previous while values
  int prevWhileCL           = whileConditionalLocation;
  int prevWhileJL           = whileJumpLocation;
  nodekind_t prevWhileType  = whileConditionalType;

  // get the start of the expression
  int startLoc = emitSkip(0);
  whileConditionalLocation = startLoc;

  // emit the expression
  inNonInlineableExp = 1;
  expression( nodePtr->ptr1, 1 );
  inNonInlineableExp = 0;

  // store away the type of conditional
  if ( nodePtr->ptr1->kind == expSimple &&
       nodePtr->ptr1->ptr1->kind == simpExpRelop) {
    whileConditionalType = nodePtr->ptr1->ptr1->ptr2->kind;
  } else {
    whileConditionalType = relopEQ;
  }

  // skip over the jump statement
  int cndJmpLoc = emitSkip(1);
  whileJumpLocation = cndJmpLoc;

  // output the statement code
  statement( nodePtr->ptr2 );

  // put an unconditional jump back to the top
  emitRM(LDC,ac0,0,0,"");
  emitRMAbs(JEQ, ac0, startLoc,
      "  if: Jump to else part if test is false (exp == 0), this is unconditional");

  // mark the end and resume to next greatest value
  int endLoc = emitSkip(0);

  // back up and write the conditional expression as the jump
  // statement
  emitBackup(cndJmpLoc);
  TreeNode* expStmtNode = nodePtr->ptr1;
  TreeNode* simpExpNode = expStmtNode->ptr1;
  if ( simpExpNode->kind == simpExpRelop ) {
    emitOppositeSelStmt(simpExpNode->ptr2->kind, ac0, endLoc);
  } else {
    emitRMAbs(JEQ,ac0, endLoc,"");
  }
  emitRestore();

  // restore the while statement values
  whileConditionalLocation  = prevWhileCL;
  whileJumpLocation         = prevWhileJL;
  whileConditionalType      = prevWhileType;
}

/* forStmt -> for '(' exp ';' exp ';' exp ')' stmt */
static void forStmt(TreeNode* nodePtr) {
  // alias each of the different pieces
  TreeNode* initAssign  = nodePtr->ptr1;
  TreeNode* conditional = nodePtr->ptr2;
  TreeNode* modifier    = nodePtr->ptr3;
  TreeNode* statements  = nodePtr->ptr4;

  // save away the previous while values
  int prevWhileCL           = whileConditionalLocation;
  int prevWhileJL           = whileJumpLocation;
  nodekind_t prevWhileType  = whileConditionalType;

  // nothing inside the forloop can be inlinable...
  inNonInlineableExp = 1;

  // emit the initial assignment
  expression( initAssign, 0 );

  // get the start of the expression
  int startLoc = emitSkip(0);
  whileConditionalLocation = startLoc;

  // emit the conditional code
  expression( conditional, 1 );

  // store away the type of conditional
  if ( conditional->kind        == expSimple &&
       conditional->ptr1->kind  == simpExpRelop ) {
    whileConditionalType = conditional->ptr1->ptr2->kind;
  } else {
    whileConditionalType = relopEQ;
  }

  // skip over the jump statement
  int cndJmpLoc = emitSkip(1);
  whileJumpLocation = cndJmpLoc;

  // the body statments can be inlined
  inNonInlineableExp = 0;

  // output the body statements
  statement( statements );

  // emit the modifier below the statement body - should be able to be inlined
  expression( modifier, 0 );

  // put an unconditional jump back to the top
  emitRM(LDC,ac0,0,0,"");
  emitRMAbs(JEQ, ac0, startLoc,
      "  if: Jump to else part if test is false (exp == 0), this is unconditional");

  // mark the end and resume to next greatest value
  int endLoc = emitSkip(0);

  // back up and write the conditional expression as the jump
  // statement
  emitBackup(cndJmpLoc);
  TreeNode* simpExpNode = conditional->ptr1;
  if ( simpExpNode->kind == simpExpRelop ) {
    emitOppositeSelStmt(simpExpNode->ptr2->kind, ac0, endLoc);
  } else {
    emitRMAbs(JEQ,ac0, endLoc,"");
  }
  emitRestore();

  // restore the while statement values
  whileConditionalLocation  = prevWhileCL;
  whileJumpLocation         = prevWhileJL;
  whileConditionalType      = prevWhileType;
}

/* 17a. jumpStmt -> break | continue */
static void jumpStmt(TreeNode* nodePtr) {
  if ( nodePtr->kind == jumpStmtBreak ) {
    // emit the proper ac0 register set so that the conditional test at the top fails
    switch ( whileConditionalType ) {
    case relopLT:
      emitRM(LDC,ac0,1,1,
             "  if: Put a 1 in ac0 so we jump over the while loop");
      break;
    case relopLE:
      emitRM(LDC,ac0,1,1,
             "  if: Put a 1 in ac0 so we jump over the while loop");
      break;
    case relopGT:
      emitRM(LDC,ac0,-1,1,
             "  if: Put a 1 in ac0 so we jump over the while loop");
      break;
    case relopGE:
      emitRM(LDC,ac0,-1,1,
             "  if: Put a 0 in ac0 so we jump over the while loop");
      break;
    case relopEQ:
      emitRM(LDC,ac0,1,1,
             "  if: Put a 1 in ac0 so we jump over the while loop");
      break;
    case relopNE:
      emitRM(LDC,ac0,0,1,
             "  if: Put a 0 in ac0 so we jump over the while loop");
      break;
    default:
      break;
    }
    emitRM(LDC,ac1,0,0,
           "  if: Put a 0 in ac0 so we jump over the else part");
    emitRMAbs(JEQ,ac1,whileJumpLocation,"  if: Jump to the jump at top of while loop");     // Jump to the jump
  }
  else { // nodePtr->kind == jumpStmtContinue
    emitRM(LDC,ac0,0,0,
           "  if: Put a 0 in ac0 so we jump over the else part");
    emitRMAbs(JEQ,ac0,whileConditionalLocation,"  if: Jump to the conditional");     // Jump to conditional
  }
}

/* 18. retStmt -> return ';' | return exp ';' */
static void returnStmt(TreeNode* nodePtr) {
    if (nodePtr->kind == retStmtVoid) {
      if ( !insideInline ) {
        emitRM(LD,pc,-1,fp,"Returning (end of function)");   // return. set PC
      }
    } else { //if (nodePtr->kind == retStmtExp)
      expression(nodePtr->ptr1, 0);       // Output exp code (ans in ac0)
      if ( !insideInline ) {
        emitRM(LD,pc,-1,fp,"Returning (end of function)");  // return. set PC
      }
    }
}

/* 19. exp -> var '=' exp | simpExp */
static void expression(TreeNode* nodePtr, int noJump) {
    if (nodePtr->kind == expAssign) {
      // don't push to the stack if there is a single var
      // can just do operation in-place
      if ( nodePtr->ptr1->kind == varSingle ) {
        expression(nodePtr->ptr2, 0);         // Output exp code (ans in ac0)
        var(nodePtr->ptr1, 0);             // Output var code (addr in ac1)
        emitRM(ST,ac0,0,ac1,"  assignment: variable = dMem[ac1] = value");
      } else {
        expression(nodePtr->ptr2, 0);         // Output exp code (ans in ac0)
        push(ac0,"  assignment: save value");
        var(nodePtr->ptr1, 0);             // Output var code (addr in ac1)
        pop(ac0,"  assignment: retrieve value");
        emitRM(ST,ac0,0,ac1,"  assignment: variable = dMem[ac1] = value");
      }
    } else //if (nodePtr->kind == expSimple) {
      simpleExp(nodePtr->ptr1, noJump);
}

/* 20. var -> ID | ID '[' exp ']' */
static void var(TreeNode* nodePtr, int rlval) {
  SemRec* semRecPtr;

  int loc, loc2;
  if (nodePtr->kind == varSingle) {
    sprintf(variableName, "%s_%s", functionName, nodePtr->value.string);
    semRecPtr = lookup(nodePtr->line, currentScope, variableName);

    if (rlval == 0) { // we want the lvalue, ac1 = var addr
      emitRM(LDA,ac1,semRecPtr->v.offset,semRecPtr->v.base,
        "  variable: AC1 = address of variable");
    }
    else {                              // we want the rvalue
      switch(semRecPtr->v.kind) {
      case intvar:                   // ac0 = var value
        emitRM(LD,ac0,semRecPtr->v.offset,semRecPtr->v.base,
               "  variable: AC0 = value of variable");
        break;
      case intarr:                   // ac0 = array addr
        emitRM(LDA,ac0,semRecPtr->v.offset,semRecPtr->v.base,
               "  variable: AC0 = address of array");
        break;
      case refarr:                   // ac0 = array addr
        emitRM(LD,ac0,semRecPtr->v.offset,semRecPtr->v.base,
               "  variable: AC0 = address of array");
        break;
      default:
        break;
      }
    }
  } else { //if (nodePtr->kind == varArray)

    expression(nodePtr->ptr1, 0);          // Output exp code

    loc = emitSkip(1);                  // Jump if subscript legal
    emitRO(HALT,0,0,0,"  variable: Stop. Neg subscripts illegal.");        // Stop. Neg subscripts illegal.
    loc2 = emitSkip(0);
    emitBackup(loc);
    emitRMAbs(JGE,ac0,loc2,"  variable: Jump if subscript >= 0");       // Jump if subscript >= 0
    emitRestore();                      // Restore instr counter

    sprintf(variableName, "%s_%s", functionName, nodePtr->value.string);
    semRecPtr = lookup(nodePtr->line, currentScope, variableName);

    if (rlval == 0) { // we want the lvalue, Get arr[exp] addr.
      if (semRecPtr->v.kind == intarr) {  // For intarr
        emitRM(LDA,ac1,semRecPtr->v.offset,semRecPtr->v.base,"");
        emitRO(SUB,ac1,ac1,ac0,"");   // (ac1 = arr[exp] addr)
      }
      else {                              // For refarr
        emitRM(LD,ac1,semRecPtr->v.offset,semRecPtr->v.base,"");
        emitRO(SUB,ac1,ac1,ac0,"");   // (ac1 = arr[exp] addr)
      }
    } else { // we want the rvalue, Get arr[exp] value
      if (semRecPtr->v.kind == intarr) {       // For intarr
        emitRM(LDA,ac1,semRecPtr->v.offset,semRecPtr->v.base,"");
        // Get arr addr
        emitRO(SUB,ac1,ac1,ac0,"");   // (ac1 = arr[exp] addr)
        emitRM(LD,ac0,0,ac1,"");      // (ac0 = arr[exp] val)
      }
      else {                                   // For refarr
        emitRM(LD,ac1,semRecPtr->v.offset,semRecPtr->v.base,"");
        emitRO(SUB,ac1,ac1,ac0,"");   // (ac1 = arr[exp] addr)
        emitRM(LD,ac0,0,ac1,"");      // (ac0 = arr[exp] val)
      }
    }
  }
}

/* 21. simpExp -> addExp relop addExp | addExp */
/* 22. relop -> '<=' | '<' | '>' | '>=' | '==' | '!=' */
static void simpleExp(TreeNode* nodePtr, int noJump) {
    if (nodePtr->kind == simpExpRelop) {
      additiveExp(nodePtr->ptr1);   // Output code for addExpNormal (ans in ac0)
      push(ac0,"");                 // Save 1st operand
      additiveExp(nodePtr->ptr3);   // Output code for addExpTerm (ans in ac0)
      pop(ac1,"");                  // Get 1st operand

      // subtract the two exps to compare them
      emitRO(SUB,ac0,ac1,ac0,""); // ac0 = addExpNormal - addExpTerm

      // if noJump flag is on, don't emit jumping code to properly set ac0
      // this is used for sel stmts and while loops to reduce num instructions
      if ( noJump ) {
        fprintf(stderr, "no jumping this time");
        return;
      }

      // assumptions
      // False: LT, GT, EQ, NE
      // True: LE, GE
      nodekind_t relop_kind = nodePtr->ptr2->kind;
      int cndJumpLoc, uncndJumpLoc, confirmedLoc, endLoc;
      if ( relop_kind == relopLT ||
           relop_kind == relopGT ||
           relop_kind == relopEQ ||
           relop_kind == relopNE ) {

        inNonInlineableExp = 1;

        // conditional jump that depends on the SUB of ac0 and ac1
        cndJumpLoc = emitSkip(1);     // Relop: jump to F or continue to T

        // sets result to be true and unconditionally
        // jumps to the end
        emitRM(LDC,ac0,1,0,"");     // ac0 = TRUE
        emitRM(LDC,ac1,0,0,"");     // set up for unconditional jump
        uncndJumpLoc = emitSkip(1);   // unconditional jump to end

        // location of opposite setting
        confirmedLoc = emitSkip(0);   // True location
        emitRM(LDC,ac0,0,0,"");     // ac0 = FALSE

        endLoc = emitSkip(0);         // End location

        inNonInlineableExp = 0;
      } else {
        // conditional jump that depends on the SUB of ac0 and ac1
        cndJumpLoc = emitSkip(1);     // Relop: jump to T or continue to F

        // sets result to be true and unconditionally
        // jumps to the end
        emitRM(LDC,ac0,0,0,"");     // ac0 = FALSE
        emitRM(LDC,ac1,0,0,"");     // set up for unconditional jump
        uncndJumpLoc = emitSkip(1);   // unconditional jump to end

        // location of opposite setting
        confirmedLoc = emitSkip(0);   // True location
        emitRM(LDC,ac0,1,0,"");     // ac0 = TRUE

        endLoc = emitSkip(0);         // End location
      }

      // write in the conditional jump, this is the hard part since we assume
      // different conditions for different comparison ops
      emitBackup(cndJumpLoc);              // Fill in relop jump instr
      if (nodePtr->ptr2->kind == relopLE)
          emitRMAbs(JLE,ac0,confirmedLoc,""); // LE
      else if (nodePtr->ptr2->kind == relopLT)
          emitRMAbs(JGE,ac0,confirmedLoc,""); // LT
      else if (nodePtr->ptr2->kind == relopGT)
          emitRMAbs(JLE,ac0,confirmedLoc,""); // GT
      else if (nodePtr->ptr2->kind == relopGE)
          emitRMAbs(JGE,ac0,confirmedLoc,""); // GE
      else if (nodePtr->ptr2->kind == relopEQ)
          emitRMAbs(JNE,ac0,confirmedLoc,""); // EQ
      else //if (nodePtr->ptr2->kind == relopNE)
          emitRMAbs(JEQ,ac0,confirmedLoc,""); // NE
      emitRestore();

      emitBackup(uncndJumpLoc);             // Fill in jump to end instr
      emitRMAbs(JEQ,ac1,endLoc,""); // Jump to end
      emitRestore();
    } else //if (nodePtr->kind == simpExpAdditive)
      additiveExp(nodePtr->ptr1);   // Output code for addExp
}

/* 23. addExp -> addExp addop term | term */
/* 24. addop -> '+' | '-' */
static void additiveExp(TreeNode* nodePtr) {
    if (nodePtr->kind == addExpNormal) {
      inNonInlineableExp = 1;

      additiveExp(nodePtr->ptr1);        // Output addExp code (ans in ac0)
      push(ac0,"");                      // Save answer
      term(nodePtr->ptr3);               // Output term code (ans in ac0)
      // 2nd operand now in ac0, no need to push/pop
      pop(ac1,"");                       // Get 1st operand

      // generate ADD operation or SUB operation depending
      // on the kind
      if(nodePtr->ptr2->kind == addop)
        emitRO(ADD,ac0,ac1,ac0,""); // ac0 = addExp + term
      else
        emitRO(SUB,ac0,ac1,ac0,""); // ac0 = addExp - term

      inNonInlineableExp = 0;

    } else //if (nodePtr->kind == addExpTerm)
      term(nodePtr->ptr1);               // Output code for term
}

/* 25. term -> term mulop factor | factor */
/* 26. mulop -> '*' | '/' */
static void term(TreeNode* nodePtr) {
    if (nodePtr->kind == termNormal) {
      inNonInlineableExp = 1;

      term(nodePtr->ptr1);                 // Output term code (ans in ac0)
      push(ac0,"");                        // Save 1st operand
      factor(nodePtr->ptr3);               // Output factor code (ans in ac0)
      // 2nd operand now in ac0, no need to push/pop
      pop(ac1,"");                         // Get 1st operand
      if(nodePtr->ptr2->kind == mulopMult)
        emitRO(MUL,ac0,ac1,ac0,"");    // ac0 = term * factor
      else
        emitRO(DIV,ac0,ac1,ac0,"");    // ac0 = term / factor

      inNonInlineableExp = 0;
    } else //if (nodePtr->kind == termFactor)
      factor(nodePtr->ptr1);               // Output code for factor
}

/* 27. factor -> '(' exp ')' | var | call | NUM | FNUM */
static void factor(TreeNode* nodePtr) {
    if (nodePtr->kind == factorExp)
      expression(nodePtr->ptr1, 0);           // Output exp code (ans in ac0)
    else if (nodePtr->kind == factorVar)
      var(nodePtr->ptr1,1);                // Output var code (rvalue in ac0)
    else if (nodePtr->kind == factorCall)
      call(nodePtr->ptr1);                 // Output call code (ans in ac0)
    else if (nodePtr->kind == factorFloat)
      emitFloatSet(ac0, nodePtr->value.floating);
    else //if (nodePtr->kind == factorNum)
      emitRM(LDC,ac0,nodePtr->value.integer,0,""); // ac0 = NUM value
}

/* 28. call -> ID '(' args ')' */
static void call(TreeNode* nodePtr) {
    // get the semantic record
    SemRec* semRecPtr;
    semRecPtr = lookup(nodePtr->line, currentScope, nodePtr->value.string);

    // only inline if the function is inlineable and the function call
    // is in an inlinable location, including not being a tail call
    if ( nodePtr->kind == call1 && !inNonInlineableExp &&
         semRecPtr->f.inlineable && strcmp(functionName, nodePtr->value.string) != 0 &&
         with_inlining) {
      if ( strcmp(nodePtr->value.string, "input") == 0 ) {
        emitRO(IN,ac0,0,0,  "     Get input");
        numInlineCalls++;
      }
      else if ( strcmp(nodePtr->value.string, "output") == 0 ) {
        args(nodePtr->ptr1);
        emitRM(LD,ac0,1,sp,"     Get output");
        emitRO(OUT,ac0,0,0, "     Give output");
        emitRM(LDA, sp, 1, sp, "");
        numInlineCalls++;
      }
      else {

        // move the frame pointer down and the stack poitner down
        int totalSubVars = semRecPtr->f.numParams + semRecPtr->f.localSpace;
        if ( totalSubVars > 0) {
          emitRM(LDA, sp, -2, sp, "");
        }

        // emit the arguments and push the stack space down to give room for vars
        args(nodePtr->ptr1);

        // move into a new scope, change the function name
        Scope* oldScope = currentScope;
        currentScope = semRecPtr->f.scope;
        char oldFunctionName[50];
        strcpy(oldFunctionName, functionName);
        strcpy(functionName, nodePtr->value.string);

        if ( totalSubVars > 0 ) {
          emitRM(ST, fp, semRecPtr->f.numParams + 2, sp, "");
          emitRM(LDA, fp, semRecPtr->f.numParams + 2, sp, " move FP up to be able to reuse fn code");
          emitRM(LDA, sp, -semRecPtr->f.localSpace, sp, "");
        }

        // emit the function code and function name
        insideInline = 1;
        functionStmt(semRecPtr->f.statements);
        insideInline = 0;

        strcpy(functionName, oldFunctionName);

        // go back to the old scope
        currentScope = oldScope;

        // move the SP back
        if ( totalSubVars > 0 ) {
          emitRM(LDA, sp, semRecPtr->f.localSpace + semRecPtr->f.numParams + 2, sp, "");
          emitRM(LD, fp, 0, fp, "");
        }

        numInlineCalls++;
      }
    } else {

      // push the FP to the stack and save space for the return address
      if ( nodePtr->kind == call1 ) {
        push(fp,              "Function call, save old FP");
        emitRM(LDA,sp,-1,sp,"     Save space for return addr");
      }

      // output argument code
      args(nodePtr->ptr1);

      // set the FP if we are in a call,
      // otherwise, copy new arguments into proper location and set SP right after them
      if ( nodePtr->kind == call1 ) {
        emitRM(LDA, fp, semRecPtr->f.numParams+2, sp,"   Set the top of the stack frame");

        // set the SP below the local variables
        emitRM(LDA,sp,-nodePtr->locals_so_far,sp, "     Set SP after locals");
      }
      else {
        for ( int n = 0; n < semRecPtr->f.numParams; n++ ) {
          emitRM(LD, ac0, semRecPtr->f.numParams-n, sp, "");
          emitRM(ST, ac0, -(n+2), fp, "");
        }

        // set the SP to right below the parameters and local varaibles
        int offset = 2 + semRecPtr->f.numParams + nodePtr->locals_so_far;
        emitRM(LDA, sp, -offset, fp,"    Set the SP after function parameters and locals");
      }

      // get the return address
      // if tail call, load from the FP
      if ( nodePtr->kind == call1 ) {
        emitRM(LDA,ac0,2,pc,"     Get return addr");  // ac0 = return addr (pc+1)
        emitRM(ST, ac0, -1, fp, "     Store the return address in position");
      }

      // jump to the function we wish to call
      char* comment = calloc(500, sizeof(char));
      sprintf(comment, "CALL %s:     Jump to function", nodePtr->value.string);
      emitRMAbs(LDA,pc,semRecPtr->f.addr, comment);
      free(comment);

      // restore the old SP and FP after calling
      if ( nodePtr->kind == call1 ) {
        emitRM(LDA,sp,0,fp,                "     Restore old SP");
        emitRM(LD,fp,0,fp,                 "     Restore old FP");
      }
    }
}

/* 29. args -> argList | empty */
static void args(TreeNode* nodePtr) {
  inNonInlineableExp = 1;

    if (nodePtr->kind == argsNormal)
      argList(nodePtr->ptr1);              // Output argList code
    //else if (nodePtr->kind == argsVoid)       // No code to generate

  inNonInlineableExp = 0;
}

/* 30. argList -> exp ',' argList | exp */
static void argList(TreeNode* nodePtr) {
    if (nodePtr->kind == argListNormal) {
      expression(nodePtr->ptr1, 0);           // Output exp code
      push(ac0,"");
      argList(nodePtr->ptr2);              // Output arglist code
    } else { //if (nodePtr->kind == argListSingle)
      expression(nodePtr->ptr1, 0);           // Output exp code
      push(ac0,"");
    }
}
