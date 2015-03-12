#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "compiler.h"

#include "support.h"
#include "semantic.h"
#include "codegen.h"
#include "profiler.h"

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
  int with_profile = 0;
  for (int n = 1; n < argc; n++) {
    if ( strcmp(argv[n], "-f") == 0 ) {
      with_float = 1;
    }
    if ( strcmp(argv[n], "-p") == 0 ) {
      with_profile = 1;
    }
  }

  fprintf(stderr,"Creating parse tree...\n");
  yyparse();

  fprintf(stderr,"Semantic checking...\n");
  semanticCheck(parseTreePtr);

  fprintf(stderr,"Generating code...\n");
  codegenSupportInit();
  generateCode(parseTreePtr);

  // profile if enabled
  if ( with_profile ) {
    fprintf(stderr, "Profiling...\n");
    profile();
  }

  fprintf(stderr, "Writing code to stdout...\n");
  printInstructionTable();

  fprintf(stderr,"Done!\n");

  return 0;
}

