#include <glib.h>

#include "compiler.h"

extern char*      myalloc(size_t size);
extern TreeNode*  newParseTreeNode();
extern SemRec*    newSemRec();
extern HashNode*  newHTableNode();
extern HashTable* newHashTable();
extern Scope*     newScope();
