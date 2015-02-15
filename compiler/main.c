#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

// Evil (but necessary) Globals!
SymbolTable   symTabPtr = NULL;  // Pointer to the top of the symbol table
ParseTree     parseTreePtr;      // Pointer to the top of the parse tree

// function declarations
int hashFunct(char* theName);
void generateCode(TreeNode* nodePt);

/****************************************************************************/
/*                                                                          */
/* main()                                                                   */
/*                                                                          */
/****************************************************************************/

int main(int argc, char** argv) {
  // parse the command line options
  with_float = 0;
  for (int n = 1; n < argc; n++) {
    if ( strcmp(argv[n], "-f") == 0 ) {
      with_float = 1;
    }
  }

  fprintf(stderr,"Creating parse tree...\n");
  yyparse();
  fprintf(stderr,"Semantic checking...\n");
  semanticCheck(parseTreePtr);
  fprintf(stderr,"Generating code...\n");
  generateCode(parseTreePtr);
  fprintf(stderr,"Done!\n");

  return 0;
}

/****************************************************************************/
/*                                                                          */
/* symbol table                                                             */
/*                                                                          */
/****************************************************************************/

SemRec* lookup(int line, SymbolTable scopePtr, char* theName) {
     int slot;
     SemRec* tempRec;
     HashNode* tempNode;

     slot = hashFunct(theName);      // Hash the key.

     // If the scope is NULL, the SemRec does not exist, error.
     if (scopePtr == NULL) {
	  fprintf(stderr, "Variable/Function does not exist.\n");
	  fprintf(stderr, "Line = %d, Name = %s \n", line, theName);
	  exit(1);
     }

     tempNode = scopePtr->theTable[slot];  // Look for the node.
     while((tempNode != NULL) && strcmp(tempNode->key,theName))
	  tempNode = tempNode->nextNode;

     if (tempNode == NULL)                 // If not found, look in next scope.
	  tempRec = lookup(line, scopePtr->prevScope, theName);
     else                                  // Else, retrieve it.
	  tempRec = tempNode->theSemRec;

     return tempRec;
}

void insert(int line, SymbolTable scopePtr, char* theName, SemRec* theRec) {
     int slot = hashFunct(theName);            // Hash the key.

     HashNode* box = newHTableNode();              // Make a node for it.

     // copy the string
     box->key = calloc(strlen(theName), sizeof(char));
     strcpy(box->key, theName);

       box->theLine = line;
     box->theSemRec = theRec;
     box->nextNode = NULL;

     box->nextNode = scopePtr->theTable[slot]; // Insert the node.
     scopePtr->theTable[slot] = box;
}

int hashFunct(char* theName) {
     int len,i, value;
     value = 0;

     len = strlen(theName);
     for (i=0;i<len;i++)
	  value += theName[i];

     return (value % M);
}

// Make a new scope and push it on the symbol table
void beginScope() {
    Scope* tempScope = newScope();              // Make a new scope
    tempScope->theTable = newHashTable();        // Make a hash table
    tempScope->prevScope = symTabPtr;               // Set the prev scope
    symTabPtr = tempScope;                          // Point to the scope
}

// Remove a scope
void endScope() {
    int i;
    HashNode* tempNodeP, * tempNodeP2;
    Scope* tempScopeP = symTabPtr;                 // Point to scope
    symTabPtr = tempScopeP->prevScope;              // Scope is prev scope

    for (i=0; i<M; i++) {                        // Destroy old hash table
	tempNodeP = tempScopeP->theTable[i];
	while (tempNodeP != NULL) {
	    free(tempNodeP->theSemRec);
	    free(tempNodeP->key);
	    tempNodeP2 = tempNodeP->nextNode;
	    free(tempNodeP);
	    tempNodeP = tempNodeP2;
	}
    }
    free(tempScopeP->theTable);

    free(tempScopeP);                            // Destroy old scope
}


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
HashNode* newHTableNode() { return (HashNode*) myalloc(sizeof(HashNode)); }

HashTable* newHashTable() {
     int i;
     HashTable* temp = (HashTable*) myalloc(M*sizeof(HashTable));
     for(i = 0; i<M; i++) temp[i]=NULL;
     return temp;
}

