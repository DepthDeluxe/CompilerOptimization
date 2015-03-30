#include "types.h"
#include "support.h"

#include "trimmer.h"

void trimAll(TreeNode* top) {
  trimAdditiveFolding(top);
}

int trimAdditiveFolding(TreeNode* top) {
  // keep recursing if we don't find the node type we are looking for
  if ( top->ptr1 != NULL ) {
    trimAdditiveFolding(top->ptr1);
  }
  if ( top->ptr2 != NULL ) {
    trimAdditiveFolding(top->ptr2);
  }
  if ( top->ptr3 != NULL ) {
    trimAdditiveFolding(top->ptr3);
  }
  if ( top->ptr4 != NULL ) {
    trimAdditiveFolding(top->ptr4);
  }

  // only return if there wasn't anything immediately below us
  if ( top->kind != addExpNormal ) {
    return 0;
  }

  // we found an additive expression, time to see if it consists of
  // two numbers added together
  TreeNode* left = top->ptr1;
  TreeNode* right = top->ptr3;

  // delve down into the tree to pull out whether or not the additive
  // expression is a number
  int leftIsNumber = 0;
  if ( left             != NULL && left->kind == addExpTerm &&
       left->ptr1       != NULL && left->ptr1->kind == termFactor &&
       left->ptr1->ptr1 != NULL && left->ptr1->ptr1->kind == factorNum ) {
    leftIsNumber = 1;
  }

  // delve down into the the tree to pull out whether or not the term
  // is a number
  int rightIsNumber = 0;
  if ( right       != NULL && right->kind == termFactor &&
       right->ptr1 != NULL && right->ptr1->kind == factorNum ) {
    rightIsNumber = 1;
  }

  // we can fold the two if both sides are numbers
  if ( leftIsNumber && rightIsNumber ) {
    int leftVal = left->ptr1->ptr1->value.integer;
    int rightVal = right->ptr1->value.integer;

    // add or subtract depending on the op provided in the file
    int sum;
    if ( top->ptr2->kind == addop ) {
      sum = leftVal + rightVal;
    } else {
      sum = leftVal - rightVal;
    }

    // produce proper structure for numeric content
    top->kind = addExpTerm;
    top->ptr2 = NULL;
    top->ptr3 = NULL;

    top->ptr1 = newParseTreeNode();
    top->ptr1->kind = termFactor;

    top->ptr1->ptr1 = newParseTreeNode();
    top->ptr1->ptr1->kind = factorNum;
    top->ptr1->ptr1->value.integer = sum;

    return 1;
  }

  return 0;
}

