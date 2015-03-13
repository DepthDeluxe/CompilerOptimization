#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "support.h"

/****************************************************************************/
/*                                                                          */
/* utilities                                                                */
/*                                                                          */
/****************************************************************************/

char* myalloc(size_t size) {
  char* temp;
  if ((temp = malloc(size)) != NULL)
    return temp;
  else {
   fprintf(stderr, "Out of memory!\n");
   fprintf(stderr, "(Successfully compiled %d lines.)\n", lineno);
   exit(1);
  }
}

TreeNode* newParseTreeNode() {
  TreeNode* temp;
  temp = (TreeNode*) myalloc(sizeof(TreeNode));
  temp->line = lineno;
  return temp;
}

Scope* newScope() { return (Scope*) myalloc(sizeof(Scope)); }
SemRec* newSemRec() { return (SemRec*) myalloc(sizeof(SemRec)); }

