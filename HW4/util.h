#ifndef _UTIL_H_
#define _UTIL_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <sys/user.h>

using namespace std;

typedef unsigned char byte;

enum State
{
    NONE,
    LOADED,
    RUNNING
};

struct Text
{
    unsigned long int offset;
    unsigned long int entry;
    unsigned long int size;
    byte *data;
};

struct Program
{
    enum State state = State::NONE;
    string path;
    char **argv;
    int pid;
    unsigned long int base_address;
    struct Text text;
    bool dynamic;
    vector<struct bp> bplist;
    struct user_regs_struct regs;
};

struct bp
{
    unsigned long long addr;
    byte ovalue;
};

void errquit(const char *msg);

byte change_byte(unsigned long int addr, byte value);

#endif