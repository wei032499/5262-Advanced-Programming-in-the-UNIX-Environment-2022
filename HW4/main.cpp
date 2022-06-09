#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fstream>

#include "util.h"
#include "load.h"
#include "ins.h"

#include <iostream>

using namespace std;

extern struct Program program;

void execscript(char *script)
{
    fstream file(script, ios::in);
    if (file.fail())
        errquit("** script file error: ");

    string line;

    while (getline(file, line))
        execins(line);

    file.close();
}

void parse_args(int argc, char *argv[], char **script)
{
    *script = NULL;

    if (argc == 1)
        return;

    int opt;
    if ((opt = getopt(argc, argv, "s:")) != -1)
    {
        switch (opt)
        {
        case 's':
            *script = new char[strlen(optarg) + 1];
            strcpy(*script, optarg);
            if (optind < argc)
            {
                program.path = argv[optind];
                program.argv = argv + (optind++);
            }
            break;
        default:
            fprintf(stderr, "usage: %s [-s script] [program]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        program.path = argv[optind];
        program.argv = argv + (optind++);
    }
}

int main(int argc, char *argv[])
{
    // 避免輸出亂序
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    char *script = NULL;
    parse_args(argc, argv, &script);

    if (program.path != "")
        load();

    if (script != NULL)
        execscript(script);
    else
    {
        string line;
        while (true)
        {
            printf("sdb> ");
            getline(cin, line);
            execins(line);
        }
    }

    return 0;
}
