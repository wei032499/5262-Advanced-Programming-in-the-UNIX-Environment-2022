#include <proc_info.h>
#include <file_info.h>
#include <filter.h>

#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <dirent.h>

#include <fstream>
#include <errno.h>

#include <sstream>
#include <sys/stat.h>

using namespace std;

#define FORMAT "%-20s\t%-10s\t%-15s\t%-8s\t%-8s\t%-10s\t%s\n"

bool is_process(const struct dirent *dirp)
{
    if (dirp->d_type != DT_DIR)
        return false;
    string d_name = dirp->d_name;
    for (char const &c : d_name)
    {
        if (std::isdigit(c) == 0)
            return false;
    }
    return true;
}

bool traverse_fd(struct proc_info *proc)
{
    struct file_info *file_info;

    DIR *dir = opendir(("/proc/" + proc->pid + "/fd").c_str());
    struct dirent *dirp;
    if (dir == NULL || (dirp = readdir(dir)) == NULL)
    {
        file_info = getfileinfo("NOFD", "/proc/" + string(proc->pid) + "/fd");
        if (!file_info)
            return false;
        proc->file_info.push_back(file_info);
    }
    else
        do
        {
            if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
                continue;

            struct stat s;
            if (lstat(("/proc/" + proc->pid + "/fd/" + dirp->d_name).c_str(), &s) == -1)
                continue;

            if ((s.st_mode & S_IREAD) && (s.st_mode & S_IWRITE))
                file_info = getfileinfo((string(dirp->d_name) + "u").c_str(), "/proc/" + string(proc->pid) + "/fd/" + dirp->d_name);
            else if (s.st_mode & S_IRUSR)
                file_info = getfileinfo((string(dirp->d_name) + "r").c_str(), "/proc/" + string(proc->pid) + "/fd/" + dirp->d_name);
            else if (s.st_mode & S_IWUSR)
                file_info = getfileinfo((string(dirp->d_name) + "w").c_str(), "/proc/" + string(proc->pid) + "/fd/" + dirp->d_name);
            else
                continue;

            if (!file_info)
                return false;

            proc->file_info.push_back(file_info);

        } while ((dirp = readdir(dir)) != NULL);

    closedir(dir);

    return true;
}

void parse_maps(struct proc_info *proc)
{
    string line;

    fstream maps;
    maps.open("/proc/" + proc->pid + "/maps", ios::in);
    while (getline(maps, line))
    {
        stringstream ss(line);
        string addr, flags, pgoff, dev, inode, name, deleted;
        ss >> addr >> flags >> pgoff >> dev >> inode >> name >> deleted;
        if (inode != "0")
        {
            bool same = false;
            for (std::vector<struct file_info *>::iterator it = proc->file_info.begin(); it != proc->file_info.end(); ++it)
                if ((*it)->node == inode)
                {
                    same = true;
                    break;
                }
            if (same)
                continue;
            struct file_info *file_info = new struct file_info;

            if (!deleted.empty())
                file_info->fd = "DEL";
            else
                file_info->fd = "mem";

            file_info->name = name;
            file_info->type = "REG";
            file_info->node = inode;

            proc->file_info.push_back(file_info);
        }
    }
    maps.close();
}

void dump_info(struct proc_info *proc, struct filter *filter)
{
    for (std::vector<struct file_info *>::iterator it = proc->file_info.begin(); it != proc->file_info.end(); ++it)
        if (valid_finfo(filter, (*it)))
            printf(FORMAT,
                   proc->command.c_str(),
                   proc->pid.c_str(),
                   proc->username.c_str(),
                   (*it)->fd.c_str(),
                   (*it)->type.c_str(),
                   (*it)->node.c_str(),
                   ((*it)->name + (*it)->error).c_str());
}

void traverse_proc(char *pid, struct filter *filter)
{
    struct proc_info *proc = getprocinfo(pid);
    if (proc == NULL || !valid_param(proc->command, &filter->command))
        return;

    struct file_info *file_info;
    file_info = getfileinfo("cwd", "/proc/" + string(pid) + "/cwd");
    if (file_info == NULL)
    {
        destroy_procinfo(proc);
        return;
    }
    proc->file_info.push_back(file_info);

    file_info = getfileinfo("rtd", "/proc/" + string(pid) + "/root");
    if (file_info == NULL)
    {
        destroy_procinfo(proc);
        return;
    }
    proc->file_info.push_back(file_info);

    file_info = getfileinfo("txt", "/proc/" + string(pid) + "/exe");
    if (file_info == NULL)
    {
        destroy_procinfo(proc);
        return;
    }
    proc->file_info.push_back(file_info);

    parse_maps(proc);

    if (!traverse_fd(proc))
    {
        destroy_procinfo(proc);
        return;
    }

    dump_info(proc, filter);
    destroy_procinfo(proc);
}

int main(int argc, char *argv[])
{
    struct filter filter = parse_filter(argc, argv);

    printf(FORMAT, "COMMAND", "PID", "USER", "FD", "TYPE", "NODE", "NAME");

    DIR *dir = opendir("/proc");
    struct dirent *dirp;
    while ((dirp = readdir(dir)) != NULL)
        if (is_process(dirp))
        {
            char *pid = dirp->d_name;
            traverse_proc(pid, &filter);
        }
    closedir(dir);
}