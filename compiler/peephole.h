#ifndef _PROFILER_H
#define _PROFILER_H

// code profiler and optimizer
void profileLDST();
void profileSTLD();
void profileSTST();
void profileLDLD();
void profileADDSUB();
void profileMULDIV();
void profileDIVMUL();
void profileLDNULL();
void profileDUP();
void profilePUSHPOP();

void peephole();

#endif

