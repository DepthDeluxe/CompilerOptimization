#ifndef _SEMANTIC_SUPPORT_H
#define _SEMANTIC_SUPPORT_H

void printNodeType(TreeNode* node);
void printNode(TreeNode* node_ptr, GList* parent_stack);

// Semantic checking support functions
extern SemRec*    lookup(int line, SymbolTable scopePtr, char* theName);
extern void       insert(int line, SymbolTable scopePtr, char* theName, SemRec* theRec);
extern void       beginScope();
extern void       endScope();

extern void       upScope();
extern void       downScope();

#endif

