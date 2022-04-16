#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    FILE *output = stderr;
    char *cmd;
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

    char *cmd_args[argc - optind + 1];

    for (int i = optind; i < argc; i++)
        cmd_args[i - optind] = argv[i];
    cmd_args[argc - optind] = NULL;

    if (execvp(cmd_args[0], cmd_args) == -1)
    {
        perror("execvp");
        exit(EXIT_FAILURE);
    }
}