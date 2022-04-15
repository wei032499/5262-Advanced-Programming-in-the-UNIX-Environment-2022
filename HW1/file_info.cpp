#include <file_info.h>

#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <sstream>

using namespace std;

void eraseSubStr(string &mainStr, const string &toErase)
{
    size_t pos = mainStr.find(toErase);
    if (pos != std::string::npos)
    {
        // If found then erase it from string
        mainStr.erase(pos, toErase.length());
    }
}

bool getname(string path, struct file_info *info)
{
    info->path = path;
    info->name = path;
    eraseSubStr(info->name, "(deleted)");

    char name[4096];
    ssize_t len;

    struct stat s;

    if ((len = readlink(path.c_str(), name, sizeof(name) - 1)) != -1)
    {
        name[len] = '\0';
        info->name = string(name);
        eraseSubStr(info->name, "(deleted)");
    }
    else if (errno == EACCES)
        info->error = " (Permission denied)";
    else if (errno != ENOENT || lstat(path.c_str(), &s) == -1)
    {
        // cout << path << " " << errno << endl;
        return false;
    }

    return true;
}

string gettype(const char *path)
{
    struct stat s;
    if (stat(path, &s) != 0)
        return "unknown";
    else if (S_ISDIR(s.st_mode))
        return "DIR";
    else if (S_ISREG(s.st_mode))
        return "REG";
    else if (S_ISCHR(s.st_mode))
        return "CHR";
    else if (S_ISFIFO(s.st_mode))
        return "FIFO";
    else if (S_ISSOCK(s.st_mode))
        return "SOCK";
    else if (s.st_mode == 384)
        return "a_inode";
    else
    {
        // cout << path << " " << s.st_mode << endl;
        return "unknown";
    }
}

ino_t getinode(const char *path)
{
    struct stat s;
    if (stat(path, &s) != 0)
        return -1;
    else
        return s.st_ino;
}

struct file_info *getfileinfo(const char *fd, string path)
{
    struct file_info *info = new struct file_info;

    info->fd = string(fd);
    if (strcmp(fd, "NOFD") != 0)
    {
        if (!getname(path, info))
        {
            // cout << "!!!" << path << endl;
            // printf("!!!!\n");
            return NULL;
        }
        stringstream inode_ss;
        ino_t inode = getinode(info->path.c_str());
        if (inode != -1)
            inode_ss << inode;

        info->type = gettype(info->path.c_str()).c_str();
        info->node = inode_ss.str().c_str();
    }
    else
    {
        info->name = path;
        eraseSubStr(info->name, "(deleted)");
        info->error = " (Permission denied)";
    }

    return info;
}