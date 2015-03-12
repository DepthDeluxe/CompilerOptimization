#include <stdio.h>

#include "types.h"
#include "compiler.h"

#include "profiler.h"
#include "codegen.h"

/*
 * Code Profiler
 */
void _nopifyInstruction(int loc) {
  TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &loc);
  inst->opCode = NOP;
  inst->a = 0;
  inst->b = 0;
  inst->c = 0;
}

void profileLDST() {
  int state = 0;
  int numIdentified = 0;
  int a,b,c;
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    if ( inst->opCode == LD ) {
      a = inst->a;
      b = inst->b;
      c = inst->c;

      state = 1;
    } else if ( inst->opCode == ST &&
                inst->a == a &&
                inst->b == b &&
                inst->c == c ) {
      if ( state == 1 ) {
        // nopify both instructions
        _nopifyInstruction(n-1);
        _nopifyInstruction(n);
        numIdentified++;
      }

      state = 0;
    } else {
      state = 0;
    }
  }

  fprintf(stderr, "LDST: %i\n", numIdentified);
}

void profileSTLD() {
  int state = 0;
  int numIdentified = 0;
  int a,b,c;
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    if ( inst->opCode == ST ) {
      a = inst->a;
      b = inst->b;
      c = inst->c;

      state = 1;
    } else if ( inst->opCode == LD &&
                inst->a == a &&
                inst->b == b &&
                inst->c == c ) {
      if ( state == 1 ) {
        // nopify both instructions
        _nopifyInstruction(n-1);
        _nopifyInstruction(n);
        numIdentified++;
      }

      state = 0;
    } else {
      state = 0;
    }
  }

  fprintf(stderr, "STLD: %i\n", numIdentified);
}

void profileSTST() {
  int state = 0;
  int numIdentified = 0;
  int b,c;
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    if ( inst->opCode == ST ) {
      b = inst->b;
      c = inst->c;

      state = 1;
    } else if ( inst->opCode == ST &&
                inst->b == b &&
                inst->c == c ) {
      if ( state == 1 ) {
        // nopify the second op
        _nopifyInstruction(n);
        numIdentified++;
      }

      state = 0;
    } else {
      state = 0;
    }
  }

  fprintf(stderr, "STST: %i\n", numIdentified);
}

void profileLDLD() {
  int state = 0;
  int numIdentified = 0;
  int b,c;
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    if ( inst->opCode == LD ) {
      b = inst->b;
      c = inst->c;

      state = 1;
    } else if ( inst->opCode == LD &&
                inst->b == b &&
                inst->c == c ) {
      if ( state == 1 ) {
        // nopify the second op
        _nopifyInstruction(n);
        numIdentified++;
      }

      state = 0;
    } else {
      state = 0;
    }
  }

  fprintf(stderr, "LDLD: %i\n", numIdentified);
}

void profileADDSUB() {
  int state = 0;
  int numIdentified = 0;
  int X,Y;
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    if ( inst->opCode == ADD &&
         inst->a == inst->b ) {
      X = inst->a;
      Y = inst->c;

      state = 1;
    } else if ( inst->opCode == SUB &&
                inst->a == X &&
                inst->b == X &&
                inst->c == Y ) {
      if ( state == 1 ) {
        // nopify the instructions
        _nopifyInstruction(n-1);
        _nopifyInstruction(n);
        numIdentified++;
      }

      state = 0;
    } else {
      state = 0;
    }
  }

  fprintf(stderr, "ADDSUB: %i\n", numIdentified);
}

void profileSUBADD() {
  int state = 0;
  int numIdentified = 0;
  int X,Y;
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    if ( inst->opCode == SUB &&
         inst->a == inst->b ) {
      X = inst->a;
      Y = inst->c;

      state = 1;
    } else if ( inst->opCode == ADD &&
                inst->a == X &&
                inst->b == X &&
                inst->c == Y ) {
      if ( state == 1 ) {
        // nopify the instructions
        _nopifyInstruction(n-1);
        _nopifyInstruction(n);
        numIdentified++;
      }

      state = 0;
    } else {
      state = 0;
    }
  }

  fprintf(stderr, "SUBADD: %i\n", numIdentified);
}

void profileMULDIV() {
  int state = 0;
  int numIdentified = 0;
  int X,Y;
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    if ( inst->opCode == MUL &&
         inst->a == inst->b ) {
      X = inst->a;
      Y = inst->c;

      state = 1;
    } else if ( inst->opCode == DIV &&
                inst->a == X &&
                inst->b == X &&
                inst->c == Y ) {
      if ( state == 1 ) {
        // nopify the instructions
        _nopifyInstruction(n-1);
        _nopifyInstruction(n);
        numIdentified++;
      }

      state = 0;
    } else {
      state = 0;
    }
  }

  fprintf(stderr, "MULDIV: %i\n", numIdentified);
}

void profileDIVMUL() {
  int state = 0;
  int numIdentified = 0;
  int X,Y;
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    if ( inst->opCode == DIV &&
         inst->a == inst->b ) {
      X = inst->a;
      Y = inst->c;

      state = 1;
    } else if ( inst->opCode == MUL &&
                inst->a == X &&
                inst->b == X &&
                inst->c == Y ) {
      if ( state == 1 ) {
        // nopify the instructions
        _nopifyInstruction(n-1);
        _nopifyInstruction(n);
        numIdentified++;
      }

      state = 0;
    } else {
      state = 0;
    }
  }

  fprintf(stderr, "DIVMUL: %i\n", numIdentified);
}

// reutrns the number of unnecessary instructions
void _nopifyPushPop(int pushStart, int popStart) {
  // nopify the PUSH instructions
  _nopifyInstruction(pushStart);
  _nopifyInstruction(pushStart + 1);

  // nopify the POP instructions
  _nopifyInstruction(popStart);
  _nopifyInstruction(popStart + 1);
}

int _pushpopMatch(int start, int X) {
  // store the number of redundant push/pops, whether or not this
  // push/pop pair should count, and how many push/pops we are deep
  int isValid = 1;
  int depth = 1;

  // iterate instructions either until the end or until we have broken out of
  // the push/pop pair
  int i;
  for ( i = start; ( i < g_hash_table_size(instructionTable) ) && (depth > 0); i++ ) {
    // get the next instruction
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &i);

    // if we see another PUSH statement, then increment the depth counter
    int compare = strncmp("PUSH: ", inst->comment, 6);
    if ( compare == 0 ) {
      depth++;
      continue;
    }

    // break out if we find a POP, successful identification
    compare = strncmp("POP: ", inst->comment, 5);
    if ( compare == 0 ) {
      depth--;
      continue;
    }

    // make sure we don't look into another function call
    compare = strncmp("CALL:", inst->comment, 5);
    if ( compare == 0 ) {
      isValid = 0;
    }

    // if there was an assignment, the current push/pop sequence doesn't count
    // keep iterating until the appropro pop statement
    if ( inst->a == X ) {
      switch ( inst->opCode ) {
      case ADD:
      case SUB:
      case MUL:
      case DIV:
      case LD:
      case LDA:
      case LDC:
      case IN:
        isValid = 0;
        break;
      default:
        break;
      }
    }
  }

  // only perform swap where it counts, if isValid == 0, that means
  // there was an instruction that overwrote that register
  if ( isValid ) {
    _nopifyPushPop(start-2, i);
  }

  return isValid;
}

// profiles the PUSH and POP operations, checks to see if we are pushing
// something that never gets assigned to before we pop.
void profilePUSHPOP() {
  int numIdentified = 0;

  // run this part from the beginning
  for ( int n = 0; n < g_hash_table_size(instructionTable); n++ ) {
    // get the next instruction
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    // recurse again if we found another PUSH instruction
    int compare = strncmp("PUSH: ", inst->comment, 6);
    if ( compare == 0 ) {
      numIdentified += _pushpopMatch(n+2, inst->a);
    }

  }

  fprintf(stderr, "PUSHPOP: %i\n", numIdentified);
}

int _pushToCopyMatch( int start, int X ) {
  int depth = 1;      // keep track of push/pop depth
  int isValid = 1;    // assume is valid until another write operation
  int i;              // keep track of position
  for ( i = start; (i > -1) && (depth > 0); i-- ) {
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &i);

    // look for matching PUSH instructions
    int compare = strncmp("PUSH: ", inst->comment, 6);
    if ( compare == 0 ) {
      depth--;
      continue;
    }

    compare = strncmp("POP: ", inst->comment, 5);
    if ( compare == 0 ) {
      depth++;
      continue;
    }

    // make sure we don't look into another function
    compare = strncmp("CALL:", inst->comment, 5);
    if ( compare == 0 ) {
      isValid = 0;
    }

    // don't count this one if we assign to the register we pop at
    if ( inst->a == X ) {
      switch ( inst->opCode ) {
      case ADD:
      case SUB:
      case MUL:
      case DIV:
      case LD:
      case LDA:
      case LDC:
      case IN:
        isValid = 0;
        break;
      default:
        break;
      }
    }
  }

  // if there was no assignment to POP register over whole time,
  // then we should perform optimization
  if ( isValid && i > -1 ) {
    // get the register to save
    int loc = i+1;
    TMInstruction* ti = (TMInstruction*)g_hash_table_lookup(instructionTable, &loc);
    int regToSave = ti->a;

    // nopify all the instructions
    _nopifyPushPop(loc, start+1);

    ti->opCode = LDA;
    ti->a = X;
    ti->b = 0;
    ti->c = regToSave;
  }

  return isValid;
}
void profilePUSHTOCOPY() {
  int numIdentified = 0;

  // start from the end since we want outside instructions to change
  // before inside instructions
  for ( int n = g_hash_table_size(instructionTable)-1; n > 0; n-- ) {
    // get the next instruction
    TMInstruction* inst = (TMInstruction*)g_hash_table_lookup(instructionTable, &n);

    int compare = strncmp("POP: ", inst->comment, 5);
    if ( compare == 0 ) {
      // get the next instruction to find the target register
      int loc = n+1;
      TMInstruction* ti = (TMInstruction*)g_hash_table_lookup(instructionTable, &loc);
      numIdentified += _pushToCopyMatch(n-1, ti->a);
    }
  }

  fprintf(stderr, "PUSHTOCOPY: %i\n", numIdentified);
}

void profile() {
  fprintf(stderr, "Profile Results ======\n");

  profileLDST();
  profileSTLD();
  profileSTST();
  profileLDLD();
  profileADDSUB();
  profileSUBADD();
  profileMULDIV();
  profileDIVMUL();
  profilePUSHPOP();
  profilePUSHTOCOPY();

  fprintf(stderr, "======================\n");
}
