#ifndef _DISASM_H_
#define _DISASM_H_

#include "util.h"
extern struct Program program;

int capstone_disasm(unsigned long int start, unsigned long int size, int num);

#endif