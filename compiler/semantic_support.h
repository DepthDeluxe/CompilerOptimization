#ifndef _SEMANTIC_SUPPORT_H
#define _SEMANTIC_SUPPORT_H

#include <glib.h>

#include "types.h"

extern Scope* currentScope;

void printNodeType(TreeNode* node);
void printNode(TreeNode* node_ptr, GList* parent_stack);

// Semantic checking support functions
SemRec*    lookup(int line, Scope* scopePtr, char* theName);
void       insert(int line, Scope* scopePtr, char* theName, SemRec* theRec);

void       beginScope();
void       endScope();

void       upScope();
void       downScope();

#endif

