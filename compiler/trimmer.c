#include <stdio.h>

#include "types.h"
#include "support.h"

#include "trimmer.h"

// keeps track of the number of folded found
int numAdditiveFound = 0;
int numMultiplicativeFound = 0;
int numParenthesisFound = 0;

// tail calls
int numSimpleTailCallFound = 0;
int numInlineTailCallFound = 0;
int numArithmeticTailCallFound = 0;

void trimAll(TreeNode* top) {
  fprintf(stderr, "==== Trimming Profile ====\n");

  trimFolding(top);
  fprintf(stderr, "Folding\n");
  fprintf(stderr, "  * Additive: %i\n", numAdditiveFound);
  fprintf(stderr, "  * Multiplicative: %i\n", numMultiplicativeFound);
  fprintf(stderr, "  * Parenthesis: %i\n", numParenthesisFound);

  trimTailCall(top);
  fprintf(stderr, "Tail Call\n");
  fprintf(stderr, "  * Simple: %i\n", numSimpleTailCallFound);
  fprintf(stderr, "  * Inline: %i\n", numInlineTailCallFound);
  fprintf(stderr, "  * Arithmetic: %i\n", numArithmeticTailCallFound);
  fprintf(stderr, "==========================\n");
}


void _trimCheckAdditive(TreeNode* top) {
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

void _trimCheckMultiplicative(TreeNode* top) {
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
void _trimCheckParenthesis(TreeNode* top) {
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

// looks for an extremely simple tail call
int _trimDetectSimpleInReturn(TreeNode* top) {
  // if the tree structure looks just like this, then we have a simple tail call
  // do the magic!!
  if ( top != NULL &&
       top->ptr1 != NULL && top->ptr1->kind == retStmtExp &&
       top->ptr1->ptr1 != NULL && top->ptr1->ptr1->kind == expSimple &&
       top->ptr1->ptr1->ptr1 != NULL && top->ptr1->ptr1->ptr1->kind == simpExpAdditive &&
       top->ptr1->ptr1->ptr1->ptr1 != NULL && top->ptr1->ptr1->ptr1->ptr1->kind == addExpTerm &&
       top->ptr1->ptr1->ptr1->ptr1->ptr1 != NULL && top->ptr1->ptr1->ptr1->ptr1->ptr1->kind == termFactor &&
       top->ptr1->ptr1->ptr1->ptr1->ptr1->ptr1 != NULL && top->ptr1->ptr1->ptr1->ptr1->ptr1->ptr1->kind == factorCall &&
       top->ptr1->ptr1->ptr1->ptr1->ptr1->ptr1->ptr1 != NULL && top->ptr1->ptr1->ptr1->ptr1->ptr1->ptr1->ptr1->kind == call1 ) {

    // XXX: re enable this line so we can do tail call optimization
    // mark that it's a simple tail call so we can process it later
    //top->ptr1->ptr1->ptr1->ptr1->ptr1->ptr1->ptr1->kind = callTail;
    return 1;
  }

  return 0;
}

// detects an arithmetic combination in a function
int _trimDetectArithmeticCombinationInReturn(TreeNode* nodePtr, int state) {
  if ( nodePtr == NULL ) return state;

  // state 0: still looking for an addExp or term
  // state 1: found one addExp/term, looking for call
  // state 2: found one call
  // state 3: found another call
  switch ( state ) {
  case 0:
    if ( nodePtr->kind == addExpNormal ||
         nodePtr->kind == termNormal ) {
      state = 1;
    }
    break;
  case 1:
    if ( nodePtr->kind == call1 ) {
      state = 2;
    }
    break;
  case 2:
    if ( nodePtr->kind == call1 ) {
      state = 3;
    }
    break;
  default:
    state = 3;
    break;
  }

  // don't look in other calls
  if ( nodePtr->kind != call1 ) {
    state = _trimDetectArithmeticCombinationInReturn(nodePtr->ptr1, state);
    state = _trimDetectArithmeticCombinationInReturn(nodePtr->ptr2, state);
    state = _trimDetectArithmeticCombinationInReturn(nodePtr->ptr3, state);
    state = _trimDetectArithmeticCombinationInReturn(nodePtr->ptr4, state);
  }

  return state;
}

// detects whether or not there is an inline call within
// a return statement
int _trimDetectInlineInReturn(TreeNode* nodePtr, int state) {
  if ( nodePtr == NULL ) return state;

  // state 0: still looking for an addExp or term
  // state 1: found addExp/term, looking for call
  // state 2: found one call, success state
  // state 3: found another call, fail state
  switch ( state ) {
  case 0:
    if ( nodePtr->kind == addExpNormal ||
         nodePtr->kind == termNormal ) {
      state = 1;
    }
    break;
  case 1:
    if ( nodePtr->kind == call1 ) {
      state = 2;
    }
    break;
  case 2:
    // go into state 3 (too many state) if we found
    // more than one function call, since state 3 error,
    // no need for more recursion
    if ( nodePtr->kind == call1 ) {
      return 3;
    }
    break;
  default:
    return 3;
  }

  // don't search inside other calls
  if ( nodePtr->kind != call1 ) {
    state = _trimDetectInlineInReturn(nodePtr->ptr1, state);
    state = _trimDetectInlineInReturn(nodePtr->ptr2, state);
    state = _trimDetectInlineInReturn(nodePtr->ptr3, state);
    state = _trimDetectInlineInReturn(nodePtr->ptr4, state);
  }

  return state;
}
// detects whether or not there is a constant return value
int _trimDetectConstantInReturn(TreeNode* nodePtr) {
  if ( nodePtr == NULL ||
       nodePtr->ptr1 == NULL ) return 0;

  TreeNode* retStmtPtr = nodePtr->ptr1;
  if ( retStmtPtr->kind == retStmtVoid ) return 0;

  // if the return statement value is a number, then true
  TreeNode* expPtr = retStmtPtr->ptr1;
  if ( expPtr != NULL && expPtr->kind == expSimple &&
       expPtr->ptr1 != NULL && expPtr->ptr1->kind == simpExpAdditive &&
       expPtr->ptr1->ptr1 != NULL && expPtr->ptr1->ptr1->kind == addExpTerm &&
       expPtr->ptr1->ptr1->ptr1 != NULL && expPtr->ptr1->ptr1->ptr1->kind == termFactor &&
       expPtr->ptr1->ptr1->ptr1->ptr1 != NULL && expPtr->ptr1->ptr1->ptr1->ptr1->kind == factorNum ) {
    return 1;
  }

  return 0;
}

GList* _trimSimpleList = NULL;
GList* _trimConstantList = NULL;
GList* _trimInlineList = NULL;
GList* _trimArithmeticCombinationList = NULL;
void _trimHardTRFind(TreeNode* nodePtr) {
  if ( nodePtr == NULL ) return;

  // only run the tests on return statements
  if ( nodePtr->kind == stmtRet ) {

    // see if it is a constant return
    int res = _trimDetectConstantInReturn(nodePtr);
    if ( res ) {
      _trimConstantList = g_list_append(_trimConstantList, nodePtr);
    } else {

      // check inline return, 2 is the success state
      res = _trimDetectInlineInReturn(nodePtr, 0);
      if ( res == 2 ) {
        _trimInlineList = g_list_append(_trimInlineList, nodePtr);
      } else {

        // check for arithmetic combo
        res = _trimDetectArithmeticCombinationInReturn(nodePtr, 0);
        if ( res == 3 ) {
          _trimArithmeticCombinationList = g_list_append(_trimArithmeticCombinationList, nodePtr);
        } else {

          // check for a simple tr
          res = _trimDetectSimpleInReturn(nodePtr);
          if ( res ) {
            _trimSimpleList = g_list_append(_trimSimpleList, nodePtr);
          }
        }
      }
    }
  }

  // crawl the tree to find matches - only delve down to the return statement level
  if ( nodePtr->kind != stmtRet ) {
    _trimHardTRFind(nodePtr->ptr1);
    _trimHardTRFind(nodePtr->ptr2);
    _trimHardTRFind(nodePtr->ptr3);
    _trimHardTRFind(nodePtr->ptr4);
  }
}

void _trimInlineTailCallInFunction(TreeNode* top) {
  // detect an inline tail call, don't do anything if we didn't find one
  _trimHardTRFind(top);

  // only do things if the length of the inline list is greater than 1
  numSimpleTailCallFound += g_list_length(_trimSimpleList);
  numInlineTailCallFound += g_list_length(_trimInlineList);
  numArithmeticTailCallFound += g_list_length(_trimArithmeticCombinationList);

  // blank out GLists when we are done using them
  _trimSimpleList = NULL;
  _trimConstantList = NULL;
  _trimInlineList = NULL;
  _trimArithmeticCombinationList = NULL;
}

void trimTailCall(TreeNode* top) {
  // the start of the declList is the ptr1 of the top
  TreeNode* nodePtr = top->ptr1;
  while ( nodePtr->ptr2 ) {
    // run InFunction function for all function declarations
    if ( nodePtr->ptr1 != NULL && nodePtr->ptr1->kind == declFun ) {
      _trimInlineTailCallInFunction(nodePtr->ptr1);
    }

    // if there are more decls in decl list, then move this again
    if ( nodePtr->ptr2 != NULL ) {
      nodePtr = nodePtr->ptr2;
    }
  }
}
