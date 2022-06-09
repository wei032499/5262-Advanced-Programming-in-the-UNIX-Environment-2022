#ifndef _INS_H_
#define _INS_H_

#include "util.h"
extern struct Program program;

#include <vector>
#include <string>
using namespace std;

void init_func();

void execins(string line);

int insbreak(vector<string> &args);
int inscont(vector<string> &args);
int insdelete(vector<string> &args);
int insdisasm(vector<string> &args);
int insdump(vector<string> &args);
int insexit(vector<string> &args);
int insget(vector<string> &args);
int insgetregs(vector<string> &args);
int inshelp(vector<string> &args);
int inslist(vector<string> &args);
int insload(vector<string> &args);
int insrun(vector<string> &args);
int insvmmap(vector<string> &args);
int insset(vector<string> &args);
int inssi(vector<string> &args);
int insstart(vector<string> &args);

#endif