#include <iostream>
#include <vector>

using namespace std;

struct proc_info
{
    string pid;
    string command;
    string username;
    vector<struct file_info *> file_info;
};

string getusername(char *pid);
string getcommand(char *pid);
struct proc_info *getprocinfo(char *pid);
void destroy_procinfo(struct proc_info *proc);