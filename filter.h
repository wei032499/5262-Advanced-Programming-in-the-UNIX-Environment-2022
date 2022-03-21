#include <iostream>
#include <vector>
#include <regex>

using namespace std;

struct filter
{
    vector<string> command;
    vector<string> type;
    vector<string> filename;
};

struct filter parse_filter(int argc, char *argv[]);
bool valid_param(string value, vector<string> *filter);
bool valid_finfo(struct filter *filter, struct file_info *info);