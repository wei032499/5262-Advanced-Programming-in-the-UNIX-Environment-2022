#include <proc_info.h>
#include <file_info.h>

#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <pwd.h>
#include <fstream>

using namespace std;

string getusername(char *pid)
{
    string path = "/proc/" + string(pid);
    struct stat sb;
    if (stat(path.c_str(), &sb) == -1)
        return "";

    struct passwd *pwd;
    pwd = getpwuid(sb.st_uid);

    return string(pwd->pw_name);
}

string getcommand(char *pid)
{
    string command;

    string path = "/proc/" + string(pid) + "/";
    fstream fin;
    fin.open(path + "comm", ios::in);
    if (!(fin >> command))
        return "";

    return command;
}

struct proc_info *getprocinfo(char *pid)
{
    struct proc_info *proc = new struct proc_info;

    proc->pid = string(pid);
    proc->command = getcommand(pid);
    proc->username = getusername(pid);
    if (proc->pid.empty() || proc->command.empty() || proc->username.empty())
        return NULL;

    return proc;
}

void destroy_procinfo(struct proc_info *proc)
{
    for (std::vector<struct file_info *>::iterator it = proc->file_info.begin(); it != proc->file_info.end(); ++it)
        delete (*it);

    delete proc;

    return;
}