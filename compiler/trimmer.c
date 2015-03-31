#include <stdio.h>

#include "types.h"
#include "support.h"

#include "trimmer.h"

// keeps track of the number of folded found
int numAdditiveFound = 0;
int numMultiplicativeFound = 0;
int numParenthesisFound = 0;

void trimAll(TreeNode* top) {
  fprintf(stderr, "==== Trimming Profile ====\n");

  trimFolding(top);
  fprintf(stderr, "Folding\n");
  fprintf(stderr, "  * Additive: %i\n", numAdditiveFound);
  fprintf(stderr, "  * Multiplicative: %i\n", numMultiplicativeFound);
  fprintf(stderr, "  * Parenthesis: %i\n", numParenthesisFound);

  fprintf(stderr, "==========================\n");
}


int _trimCheckAdditive(TreeNode* top) {
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

    // increment the profile counter for number additive found
    numAdditiveFound++;
  }
}

int _trimCheckMultiplicative(TreeNode* top) {
  // we found an additive expression, time to see if it consists of
  // two numbers added together
  TreeNode* left = top->ptr1;
  TreeNode* right = top->ptr3;

  int leftIsNum = 0;
  if ( left       != NULL && left->kind == termFactor &&
       left->ptr1 != NULL && left->ptr1->kind == factorNum ) {
    leftIsNum = 1;
  }

  int rightIsNum = 0;
  if ( right != NULL && right->kind == factorNum ) {
    rightIsNum = 1;
  }

  if ( leftIsNum && rightIsNum ) {
    int leftVal = left->ptr1->value.integer;
    int rightVal = right->value.integer;

    // compute the product of the two numbers
    int product;
    if ( top->ptr2->kind == mulopMult ) {
      product = leftVal * rightVal;
    } else {
      product = leftVal / rightVal;
    }

    top->kind = termFactor;

    top->ptr1 = newParseTreeNode();
    top->ptr1->kind = factorNum;
    top->ptr1->value.integer = product;

    // increment the profile counter for number multiplicative found
    numMultiplicativeFound++;
  }
}

// since this one dives deep enough, use a state machine
int _trimCheckParenthesis(TreeNode* top) {
  // if this clusterfuck is true, then we have something that can be removed
  if ( top != NULL && top->kind == factorExp &&
       top->ptr1 != NULL && top->ptr1->kind == expSimple &&
       top->ptr1->ptr1 != NULL && top->ptr1->ptr1->kind == simpExpAdditive &&
       top->ptr1->ptr1->ptr1 != NULL && top->ptr1->ptr1->ptr1->kind == addExpTerm &&
       top->ptr1->ptr1->ptr1->ptr1 != NULL && top->ptr1->ptr1->ptr1->ptr1->kind == termFactor &&
       top->ptr1->ptr1->ptr1->ptr1->ptr1 != NULL && top->ptr1->ptr1->ptr1->ptr1->ptr1->kind == factorNum ) {
    // get the number value
    int value = top->ptr1->ptr1->ptr1->ptr1->ptr1->value.integer;

    top->kind = factorNum;
    top->value.integer = value;

    top->ptr1 = NULL;
    top->ptr2 = NULL;
    top->ptr3 = NULL;
    top->ptr4 = NULL;

    // increment the counter for number of parenthesis found
    numParenthesisFound++;
  }
}

void trimFolding(TreeNode* top) {
  // keep recursing if we don't find the node type we are looking for
  if ( top->ptr1 != NULL ) {
    trimFolding(top->ptr1);
  }
  if ( top->ptr2 != NULL ) {
    trimFolding(top->ptr2);
  }
  if ( top->ptr3 != NULL ) {
    trimFolding(top->ptr3);
  }
  if ( top->ptr4 != NULL ) {
    trimFolding(top->ptr4);
  }

  // only return if there wasn't anything immediately below us
  if ( top->kind == addExpNormal ) {
    _trimCheckAdditive(top);
  } else if ( top->kind == termNormal ) {
    _trimCheckMultiplicative(top);
  } else if ( top->kind == factorExp ) {
    _trimCheckParenthesis(top);
  }
}

