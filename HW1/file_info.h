#include <iostream>
using namespace std;

struct file_info
{
    string path;
    string error;
    string name;

    string fd;
    string type;
    string node;
};

bool getname(string path, struct file_info *info);
string gettype(const char *path);
ino_t getinode(const char *path);
struct file_info *getfileinfo(const char *fd, string path);