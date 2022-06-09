#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <list>
#include <string>

#include "util.h"

using namespace std;

struct Program program;
extern pid_t child;

void errquit(const char *msg)
{
    perror(msg);
    exit(-1);
}

byte change_byte(unsigned long int addr, byte value)
{
    long ovalue = ptrace(PTRACE_PEEKTEXT, program.pid, addr, 0);

    /* set break point */
    if (ptrace(PTRACE_POKETEXT, program.pid, addr, (ovalue & 0xffffffffffffff00) | value) != 0)
        errquit("** ptrace(POKETEXT)");

    return (byte)ovalue;
}
