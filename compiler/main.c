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
#include "peephole.h"

ParseTree     parseTreePtr;      // Pointer to the top of the parse tree

// compiler options
int with_peephole = 0;
int with_float = 0;
int with_tree_trimming = 0;
int with_inlining = 0;

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
      with_peephole = 1;
    }
    if( strcmp(argv[n], "-t") == 0 ) {
      with_tree_trimming = 1;
    }
    if ( strcmp(argv[n], "-i") == 0 ) {
      with_inlining = 1;
    }
  }

  fprintf(stderr, "Creating parse tree...\n");
  yyparse();

  fprintf(stderr, "Semantic checking...\n");
  semanticCheck(parseTreePtr);

  if ( with_tree_trimming ) {
    fprintf(stderr, "Performing tree trimming optimizations...\n");
    trimAll(parseTreePtr);
  }

  fprintf(stderr,"Generating code...\n");
  codegenSupportInit();
  generateCode(parseTreePtr);

  // profile if enabled
  if ( with_peephole ) {
    fprintf(stderr, "Peepholing...\n");
    peephole();
  }

  fprintf(stderr, "Writing code to stdout...\n");
  printInstructionTable();

  fprintf(stderr,"Done!\n");

  return 0;
}

