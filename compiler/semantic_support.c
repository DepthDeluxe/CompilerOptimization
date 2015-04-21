#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "semantic_support.h"

#include "types.h"
#include "semantic.h"

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
  // return silently if the node_ptr is NULL
  if ( node_ptr == NULL ) {
    return;
  }

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
  g_list_remove( parent_stack, node_ptr );
}

/*
 * Scope Management
 */

Scope* currentScope;

SemRec* lookup(int line, Scope* scopePtr, char* theName) {
  SemRec* record;

  // if we are all the way up, remove function prefix by moving char pointer up
  if ( scopePtr->prevScope == NULL ) {
    for ( int n = 0; theName[n] != 0; n++ ) {
      if ( theName[n] == '_' ) {
        theName = theName + n;
      }
    }
  }

  // If the scope is NULL, the SemRec does not exist, error.
  if (scopePtr == NULL) {
    fprintf(stderr, "Variable/Function does not exist.\n");
    fprintf(stderr, "Line = %d, Name = %s \n", line, theName);
    exit(1);
  }

  GHashTable* table = scopePtr->theTable;

  record = (SemRec*)g_hash_table_lookup(table, theName);

  // lookup the next scope if the temp node is NULL
  if ( record == NULL ) {
    record = lookup(line, scopePtr->prevScope, theName);
  }

  return record;
}

void insert(int line, Scope* scope, char* theName, SemRec* theRec) {
  // copy the key to the heap
  char* heapName = calloc(strlen(theName)+1, sizeof(char));
  strcpy(heapName, theName);

  // TODO: check to see if there is a duplicate definition

  g_hash_table_replace(scope->theTable, heapName, theRec);
}

// to determine equality of string keys
gboolean _keyEqualFn(gconstpointer a, gconstpointer b) {
  int res = strcmp((char*)a, (char*)b);

  return (res==0);
}

// Make a new scope and push it on the symbol table
void beginScope() {
  // create a new scope
  Scope* newScope = (Scope*)calloc(1, sizeof(Scope));

  // create a new hash table that defines the scope
  newScope->theTable = g_hash_table_new_full( g_str_hash,
                                              _keyEqualFn,
                                              free,
                                              free );
  newScope->prevScope = currentScope;

  // new scope becomes the current scope
  currentScope = newScope;
}

// Remove a scope
void endScope() {
  // the previous scope is now the current scope
  Scope* oldScope = currentScope;
  currentScope = currentScope->prevScope;

  // free up memory used in the hash table
  g_hash_table_destroy(oldScope->theTable);

  // free up the malloced scope
  free(oldScope);
}
