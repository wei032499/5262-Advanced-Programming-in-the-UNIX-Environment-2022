#include <file_info.h>
#include <filter.h>

#include <iostream>
#include <vector>
#include <unistd.h>
#include <algorithm>

#include <regex>

using namespace std;

struct filter parse_filter(int argc, char *argv[])
{
    struct filter filter;
    vector<string> valid_types{"REG", "CHR", "DIR", "FIFO", "SOCK", "unknown"};
    int opt;
    while ((opt = getopt(argc, argv, "c:t:f:")) != -1)
    {
        switch (opt)
        {
        case 'c':
            filter.command.push_back(optarg);
            break;
        case 't':
            if (find(valid_types.begin(), valid_types.end(), optarg) != valid_types.end())
                filter.type.push_back(optarg);
            else
            {
                cout << "Invalid TYPE option" << endl;
                exit(EXIT_FAILURE);
            }
            break;
        case 'f':
            filter.filename.push_back(optarg);
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-c command line] [-t TYPE] [-c filenames]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    return filter;
}

bool valid_param(string value, vector<string> *filter)
{
    for (std::vector<string>::iterator it = filter->begin(); it != filter->end(); ++it)
    {
        regex reg(*it);
        smatch m;
        if (regex_search(value, m, reg))
        {
            /*std::cout << "regex_search: " << std::endl;

            for (auto &match : m)
            {
                sm = match;
                std::cout << sm.str() << std::endl;
            }*/

            return true;
        }
    }

    return filter->size() == 0;
}

bool valid_finfo(struct filter *filter, struct file_info *info)
{
    return valid_param(info->type, &filter->type) &&
           valid_param(info->name, &filter->filename);
}