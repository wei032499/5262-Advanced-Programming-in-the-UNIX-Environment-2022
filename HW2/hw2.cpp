#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[])
{
    FILE *output = stderr;
    char *cmd;
    vector<char *> cmd_args_vec;
    setenv("LD_PRELOAD", "./logger.so", 1);

    if (argc <= 1)
    {
        fprintf(stderr, "no command given.\n");
        exit(EXIT_FAILURE);
    }

    int opt;
    while ((opt = getopt(argc, argv, "o:p:")) != -1)
    {
        switch (opt)
        {
        case 'o':
            output = fopen(optarg, "w");
            if (!output)
            {
                perror("fopen");
                exit(EXIT_FAILURE);
            }
            break;
        case 'p':
            setenv("LD_PRELOAD", optarg, 1);
            break;
        default: /* '?' */
            fprintf(stderr, "usage: %s [-o file] [-p sopath] [--] cmd [cmd args ...]\n", argv[0]);
            fprintf(stderr, "\t-p: set the path to logger.so, default = ./logger.so\n");
            fprintf(stderr, "\t-o: print output to file, print to \"stderr\" if no file specified\n");
            fprintf(stderr, "\t--: separate the arguments for logger and for the command\n");
            exit(EXIT_FAILURE);
        }
    }

    int output_fd = fileno(output);
    output_fd = dup(output_fd);
    if (output_fd == -1)
    {
        perror("dup");
        exit(EXIT_FAILURE);
    }

    char buffer[100];
    sprintf(buffer, "%d", output_fd);

    setenv("output_fd", buffer, 1);

    for (int i = optind; i < argc; i++)
        cmd_args_vec.push_back(argv[i]);

    char *cmd_args[cmd_args_vec.size() + 1];
    copy(cmd_args_vec.begin(), cmd_args_vec.end(), cmd_args);

    cmd_args[cmd_args_vec.size()] = NULL;

    if (execvp(cmd_args[0], cmd_args) == -1)
    {
        perror("execvp");
        exit(EXIT_FAILURE);
    }
}