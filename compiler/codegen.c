#include <stdio.h> // fprintf(), stderr
//#include <stdlib.h> // atoi, atof, malloc, sizeof, ...
//#include <string.h> // c-string (char *) functions
#include "compiler.h"

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

/****************************************************************************/
/*                                                                          */
/* code generation routines                                                 */
/*                                                                          */
/****************************************************************************/
void generateCode(TreeNode* nodePtr) {
    program(nodePtr);
}

/* 1. prog -> declList */
static void program(TreeNode* nodePtr) {
    SemRec* semRecPtr;
    int loc, main1, main2;

    symTabPtr = nodePtr->symTabPtr;       // Get global symbol table

    // Run-time environment setup instructions
    emitRM("LD",gp,0,ac0,"Set GP");                // GP = dMem[0] = size-1
    loc = emitSkip(2);                             // Init FP,SP here later
    emitRM("ST",ac0,0,ac0,"Clear dMem[0]");        // re-init dMem[0] = 0
    push(fp,"Save FP");                            // dMem[sp] = fp
    main1 = emitSkip(1);                           // Re-Init SP here later
    emitRM("LDA",ac0,1,pc,"Get PC for Halt");      // ac0 has PC for HALT line
    main2 = emitSkip(1);                           // Init PC here
    emitRO("HALT",0,0,0,"Halt");                   // Stop.

    semRecPtr = lookup(nodePtr->line, symTabPtr, "input");
    semRecPtr->f.addr = emitSkip(0);
    emitRM("ST",ac0,-1,fp,"Input Function, store return addr");
    emitRO("IN",ac0,0,0,  "     Get input");
    emitRM("LD",pc,-1,fp, "     Return (end of function)");

    semRecPtr = lookup(nodePtr->line, symTabPtr, "output");
    semRecPtr->f.addr = emitSkip(0);
    emitRM("ST",ac0,-1,fp,"Output Function, store return addr");
    emitRM("LD",ac0,-2,fp,"     Get output");
    emitRO("OUT",ac0,0,0, "     Give output");
    emitRM("LD",pc,-1,fp, "     Return (end of function)");

    declarationList(nodePtr->ptr1);                  // Code for declList

    emitBackup(loc);
    emitRM("LDA",fp,-symTabPtr->used,gp,"Set FP below globals");
    emitRM("LDA",sp,-symTabPtr->used,gp,"Set SP below globals");
    emitRestore();
    semRecPtr = lookup(nodePtr->line, symTabPtr, "main");
    emitBackup(main2);
    emitRMAbs("LDA",pc,semRecPtr->f.addr,"Jump to main");// PC = main
    emitRestore();
    emitBackup(main1);
    emitRM("LDA",sp,-(semRecPtr->f.localSpace+1),sp,"Set SP below locals");
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

    semRecPtr = lookup(nodePtr->line, symTabPtr, nodePtr->value.string);
    semRecPtr->f.addr = emitSkip(0);           // Func code starts here
    symTabPtr = nodePtr->symTabPtr;                  // Get current symbol table
    params(nodePtr->ptr2);                     // Output params code
    emitRM("ST",ac0,-1,fp,"Function: store return address in dMem[FP-1]");
    functionStmt(nodePtr->ptr3);               // Output compStmt code
    emitRM("LD",pc,-1,fp,"End of function (set PC = return addr)");
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
    else //if (nodePtr->kind == stmtRet)
      returnStmt(nodePtr->ptr1);       // Output code for retStmt
}

/* 14. expStmt -> exp ';' | ';' */
static void expressionStmt(TreeNode* nodePtr) {
    if (nodePtr->kind == expStmtNormal)
      expression(nodePtr->ptr1);        // Output code for exp
    //else if (nodePtr->kind == expStmtVoid) // No code to generate
}

/* 15. compStmt -> '{' localDecl stmtList '}' */
static void compoundStmt(TreeNode* nodePtr) {
    symTabPtr = nodePtr->symTabPtr;           // Get current symbol table
    localDeclaration(nodePtr->ptr1);    // Output code for localDecl
    statementList(nodePtr->ptr2);       // Output code for stmtList
    endScope();                         // Pop scope
}

/* 16. selStmt -> if '(' exp ')' stmt | if '(' exp ')' stmt else stmt */
/* 16a. e_selStmt -> if '(' exp ')' e_stmt else stmt */
static void selectionStmt(TreeNode* nodePtr) {
    int loc, loc2, loc3, loc4;
    if (nodePtr->kind == selStmtIf) {
      expression(nodePtr->ptr1);     // Output exp code
      loc = emitSkip(1);             // Save space for "if" test
      statement(nodePtr->ptr2);      // Output stmt code

      loc2 = emitSkip(0);            // Get jump destination (end)
      emitBackup(loc);               // Backup numbering to "if" test
      emitRMAbs("JEQ",ac0,loc2,
          "  if test: Jump to end if false (exp == 0)");
      emitRestore();                 // Restore numbering
    } else { //if (nodePtr->kind == selStmtIfElse)

      expression(nodePtr->ptr1);        // Output exp code
      loc = emitSkip(1);                // Skip jump1 loc
      statement(nodePtr->ptr2);         // Output stmtExp code

      emitRM("LDC",ac0,0,0,
             "  if: Put a 0 in ac0 so we jump over the else part");
      loc2 = emitSkip(1);               // Skip jump2 loc
      loc3 = emitSkip(0);               // Get jump1 destination (else)
      statement(nodePtr->ptr3);         // Output stmtComp code
      loc4 = emitSkip(0);               // Get jump2 destination (end)

      emitBackup(loc);                  // Go back to jump1 loc
      emitRMAbs("JEQ",ac0,loc3,
          "  if: Jump to else part if test is false (exp == 0)");
      emitRestore();                    // Restore instr counter to else

      emitBackup(loc2);                 // Go back to jump2 loc
      emitRMAbs("JEQ",ac0,loc4,"  if: Jump to the end");     // Jump to end
      emitRestore();                    // Restore instr counter to end
    }
}

/* 18. retStmt -> return ';' | return exp ';' */
static void returnStmt(TreeNode* nodePtr) {
    if (nodePtr->kind == retStmtVoid) {
      emitRM("LD",pc,-1,fp,"Returning (end of function)");   // return. set PC
    } else { //if (nodePtr->kind == retStmtExp)
      expression(nodePtr->ptr1);       // Output exp code (ans in ac0)
      emitRM("LD",pc,-1,fp,"Returning (end of function)");  // return. set PC
    }
}

/* 19. exp -> var '=' exp | simpExp */
static void expression(TreeNode* nodePtr) {
    if (nodePtr->kind == expAssign) {
      expression(nodePtr->ptr2);         // Output exp code (ans in ac0)
      push(ac0,"  assignment: save value");
      var(nodePtr->ptr1, 0);             // Output var code (addr in ac1)
      pop(ac0,"  assignment: retrieve value");
      emitRM("ST",ac0,0,ac1,"  assignment: variable = dMem[ac1] = value");
    } else //if (nodePtr->kind == expSimple) {
      simpleExp(nodePtr->ptr1);
}

/* 20. var -> ID | ID '[' exp ']' */
static void var(TreeNode* nodePtr, int rlval) {
  SemRec* semRecPtr;
  int loc, loc2;
  if (nodePtr->kind == varSingle) {
    semRecPtr = lookup(nodePtr->line, symTabPtr, nodePtr->value.string);

    if (rlval == 0) { // we want the lvalue, ac1 = var addr
      emitRM("LDA",ac1,semRecPtr->v.offset,semRecPtr->v.base,
        "  variable: AC1 = address of variable");
    }
    else {                              // we want the rvalue
      switch(semRecPtr->v.kind) {
      case intvar:                   // ac0 = var value
        emitRM("LD",ac0,semRecPtr->v.offset,semRecPtr->v.base,
               "  variable: AC0 = value of variable");
        break;
      case intarr:                   // ac0 = array addr
        emitRM("LDA",ac0,semRecPtr->v.offset,semRecPtr->v.base,
               "  variable: AC0 = address of array");
        break;
      case refarr:                   // ac0 = array addr
        emitRM("LD",ac0,semRecPtr->v.offset,semRecPtr->v.base,
               "  variable: AC0 = address of array");
        break;
      default:
        break;
      }
    }
  } else { //if (nodePtr->kind == varArray)

    expression(nodePtr->ptr1);          // Output exp code

    loc = emitSkip(1);                  // Jump if subscript legal
    emitRO("HALT",0,0,0,"  variable: Stop. Neg subscripts illegal.");        // Stop. Neg subscripts illegal.
    loc2 = emitSkip(0);
    emitBackup(loc);
    emitRMAbs("JGE",ac0,loc2,"  variable: Jump if subscript >= 0");       // Jump if subscript >= 0
    emitRestore();                      // Restore instr counter

    semRecPtr = lookup(nodePtr->line, symTabPtr, nodePtr->value.string);

    if (rlval == 0) { // we want the lvalue, Get arr[exp] addr.
      if (semRecPtr->v.kind == intarr) {  // For intarr
        emitRM("LDA",ac1,semRecPtr->v.offset,semRecPtr->v.base,"");
        emitRO("SUB",ac1,ac1,ac0,"");   // (ac1 = arr[exp] addr)
      }
      else {                              // For refarr
        emitRM("LD",ac1,semRecPtr->v.offset,semRecPtr->v.base,"");
        emitRO("SUB",ac1,ac1,ac0,"");   // (ac1 = arr[exp] addr)
      }
    } else { // we want the rvalue, Get arr[exp] value
      if (semRecPtr->v.kind == intarr) {       // For intarr
        emitRM("LDA",ac1,semRecPtr->v.offset,semRecPtr->v.base,"");
        // Get arr addr
        emitRO("SUB",ac1,ac1,ac0,"");   // (ac1 = arr[exp] addr)
        emitRM("LD",ac0,0,ac1,"");      // (ac0 = arr[exp] val)
      }
      else {                                   // For refarr
        emitRM("LD",ac1,semRecPtr->v.offset,semRecPtr->v.base,"");
        emitRO("SUB",ac1,ac1,ac0,"");   // (ac1 = arr[exp] addr)
        emitRM("LD",ac0,0,ac1,"");      // (ac0 = arr[exp] val)
      }
    }
  }
}

/* 21. simpExp -> addExp relop addExp | addExp */
/* 22. relop -> '<=' | '<' | '>' | '>=' | '==' | '!=' */
static void simpleExp(TreeNode* nodePtr) {
    int loc, loc2, loc3, loc4;
    if (nodePtr->kind == simpExp1) {
      additiveExp(nodePtr->ptr1);   // Output code for addExpNormal (ans in ac0)
      push(ac0,"");                 // Save 1st operand
      additiveExp(nodePtr->ptr3);   // Output code for addExpTerm (ans in ac0)
      pop(ac1,"");                  // Get 1st operand

      emitRO("SUB",ac0,ac1,ac0,""); // ac0 = addExpNormal - addExpTerm
      loc = emitSkip(1);            // Relop: jump to T or continue to F
      emitRM("LDC",ac0,0,0,"");     // ac0 = FALSE
      emitRM("LDC",ac1,0,0,"");
      loc2 = emitSkip(1);           // Jump to end
      loc3 = emitSkip(0);           // True location
      emitRM("LDC",ac0,1,0,"");     // ac0 = TRUE
      loc4 = emitSkip(0);           // End location

      emitBackup(loc);              // Fill in relop jump instr
      if (nodePtr->ptr2->kind == relopLE)
          emitRMAbs("JLE",ac0,loc3,""); // LE
      else if (nodePtr->ptr2->kind == relopLT)
          emitRMAbs("JLT",ac0,loc3,""); // LT
      else if (nodePtr->ptr2->kind == relopGT)
          emitRMAbs("JGT",ac0,loc3,""); // GT
      else if (nodePtr->ptr2->kind == relopGE)
          emitRMAbs("JGE",ac0,loc3,""); // GT
      else if (nodePtr->ptr2->kind == relopEQ)
          emitRMAbs("JEQ",ac0,loc3,""); // EQ
      else //if (nodePtr->ptr2->kind == relopNE)
          emitRMAbs("JNE",ac0,loc3,""); // NE
      emitRestore();

      emitBackup(loc2);             // Fill in jump to end instr
      emitRMAbs("JEQ",ac1,loc4,""); // Jump to end
      emitRestore();
    } else //if (nodePtr->kind == simpExp2)
      additiveExp(nodePtr->ptr1);   // Output code for addExp
}

/* 23. addExp -> addExp addop term | term */
/* 24. addop -> '+' | '-' */
static void additiveExp(TreeNode* nodePtr) {
    if (nodePtr->kind == addExpNormal) {
      additiveExp(nodePtr->ptr1);        // Output addExp code (ans in ac0)
      push(ac0,"");                      // Save answer
      term(nodePtr->ptr3);               // Output term code (ans in ac0)
      // 2nd operand now in ac0, no need to push/pop
      pop(ac1,"");                       // Get 1st operand

      // generate ADD operation or SUB operation depending
      // on the kind
      if(nodePtr->ptr2->kind == addop)
        emitRO("ADD",ac0,ac1,ac0,""); // ac0 = addExp + term
      else
        emitRO("SUB",ac0,ac1,ac0,""); // ac0 = addExp - term

    } else //if (nodePtr->kind == addExpTerm)
      term(nodePtr->ptr1);               // Output code for term
}

/* 25. term -> term mulop factor | factor */
/* 26. mulop -> '*' | '/' */
static void term(TreeNode* nodePtr) {
    if (nodePtr->kind == termNormal) {
      term(nodePtr->ptr1);                 // Output term code (ans in ac0)
      push(ac0,"");                        // Save 1st operand
      factor(nodePtr->ptr3);               // Output factor code (ans in ac0)
      // 2nd operand now in ac0, no need to push/pop
      pop(ac1,"");                         // Get 1st operand
      if(nodePtr->ptr2->kind == mulopMult)
        emitRO("MUL",ac0,ac1,ac0,"");    // ac0 = term * factor
      else
        emitRO("DIV",ac0,ac1,ac0,"");    // ac0 = term / factor
    } else //if (nodePtr->kind == termFactor)
      factor(nodePtr->ptr1);               // Output code for factor
}

/* 27. factor -> '(' exp ')' | var | call | NUM | FNUM */
static void factor(TreeNode* nodePtr) {
    if (nodePtr->kind == factorExp)
      expression(nodePtr->ptr1);           // Output exp code (ans in ac0)
    else if (nodePtr->kind == factorVar)
      var(nodePtr->ptr1,1);                // Output var code (rvalue in ac0)
    else if (nodePtr->kind == factorCall)
      call(nodePtr->ptr1);                 // Output call code (ans in ac0)
    else if (nodePtr->kind == factorFloat)
      emitFloatSet(ac0, nodePtr->value.floating);
    else //if (nodePtr->kind == factorNum)

      emitRM("LDC",ac0,nodePtr->value.integer,0,""); // ac0 = NUM value
}

/* 28. call -> ID '(' args ')' */
static void call(TreeNode* nodePtr) {
    SemRec* semRecPtr;

    semRecPtr = lookup(nodePtr->line, symTabPtr, nodePtr->value.string);
    push(fp,              "     Function call, save old FP");
    emitRM("LDA",sp,-1,sp,"     Save space for return addr");

    args(nodePtr->ptr1);                     // Output args code

    emitRM("LDA",fp,semRecPtr->f.numParams+2,sp,"     Set FP to top of frame");
    emitRM("LDA",sp,-nodePtr->locals_so_far,sp, "     Set SP after locals");

    emitRM("LDA",ac0,1,pc,"     Get return addr");  // ac0 = return addr (pc+1)
    emitRMAbs("LDA",pc,semRecPtr->f.addr,"     Jump to function");
    emitRM("LDA",sp,0,fp,                "     Restore old SP");
    emitRM("LD",fp,0,fp,                 "     Restore old FP");
}

/* 29. args -> argList | empty */
static void args(TreeNode* nodePtr) {
    if (nodePtr->kind == argsNormal)
      argList(nodePtr->ptr1);              // Output argList code
    //else if (nodePtr->kind == argsVoid)       // No code to generate
}

/* 30. argList -> exp ',' argList | exp */
static void argList(TreeNode* nodePtr) {
    if (nodePtr->kind == argListNormal) {
      expression(nodePtr->ptr1);           // Output exp code
      push(ac0,"");
      argList(nodePtr->ptr2);              // Output arglist code
    } else { //if (nodePtr->kind == argListSingle)
      expression(nodePtr->ptr1);           // Output exp code
      push(ac0,"");
    }
}



