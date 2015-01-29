#include <stdio.h>
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
  case localDecl1:
  case localDecl2:
    type = "ldc";
    break;
  case stmtList1:
  case stmtList2:
    type = "stl";
    break;
  case funStmt1:
    type = "fns";
    break;
  case stmt1:
  case stmt2:
  case stmt3:
  case stmt4:
  case stmt5:
    type = "stm";
    break;
  case expStmt1:
  case expStmt2:
    type = "xst";
    break;
  case selStmt1:
  case selStmt2:
    type = "sst";
    break;
  case iterStmt1:
    type = "ist";
    break;
  case retStmt1:
  case retStmt2:
    type = "rst";
    break;
  case exp1:
  case exp2:
    type = "exp";
    break;
  case var1:
  case var2:
    type = "var";
    break;
  case simpExp1:
  case simpExp2:
    type = "sxp";
    break;
  case relop1:
  case relop2:
  case relop3:
  case relop4:
  case relop5:
  case relop6:
    type = "rop";
    break;
  case addExp1:
  case addExp2:
    type = "axp";
    break;
  case addop:
    type = "aop";
    break;
  case subop:
    type = "sop";
    break;
  case term1:
  case term2:
    type = "trm";
    break;
  case mulop1:
  case mulop2:
    type = "mop";
    break;
  case factor1:
  case factor2:
  case factor3:
  case factor4:
    type = "fct";
    break;
  case call1:
    type = "cal";
    break;
  case args1:
  case args2:
    type = "ags";
    break;
  case argList1:
  case argList2:
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

