#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "support.h"

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
  codegenSupportInit();
  generateCode(parseTreePtr);
  printInstructionTable();

  fprintf(stderr, "Profiling...\n");
  profile();

  fprintf(stderr,"Done!\n");

  return 0;
}

