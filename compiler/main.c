#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "compiler.h"

#include "support.h"
#include "semantic.h"
#include "trimmer.h"
#include "codegen.h"
#include "codegen_support.h"
#include "profiler.h"

ParseTree     parseTreePtr;      // Pointer to the top of the parse tree

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

  fprintf(stderr, "Creating parse tree...\n");
  yyparse();

  fprintf(stderr, "Semantic checking...\n");
  semanticCheck(parseTreePtr);

  fprintf(stderr, "Performing tree trimming optimizations...\n");
  trimAll(parseTreePtr);

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

