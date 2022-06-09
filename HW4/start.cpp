#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include "start.h"

unsigned long int get_base_address()
{
    char path[4097];
    snprintf(path, 4097, "/proc/%d/stat", program.pid);
    FILE *f = fopen(path, "r");
    if (f == NULL)
    {
        perror("** fopen error: ");
        return -1;
    }
    char dump[1024];
    for (int i = 0; i < 25; i++)
        fscanf(f, "%s", dump);

    unsigned long int address;
    fscanf(f, "%lu", &address); // startcode
    fclose(f);

    return address;
}

int start()
{
    if (program.state < LOADED)
    {
        fprintf(stderr, "** program is NOT loaded yet.\n");
        return -1;
    }
    else if (program.state > LOADED)
    {
        fprintf(stderr, "** program is running.\n");
        return -1;
    }

    program.pid = fork();

    if (program.pid == 0) // child process
    {
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0)
        {
            perror("** start ptrace error ");
            return 0;
        }
        if (execv(program.path.c_str(), program.argv) == -1)
        {
            perror("** execvp error ");
            exit(0);
        }
    }
    else // parent
    {
        int status;
        waitpid(program.pid, &status, 0);
        ptrace(PTRACE_SETOPTIONS, program.pid, 0, PTRACE_O_EXITKILL);
        program.state = RUNNING;

        program.base_address = program.dynamic ? get_base_address() : 0;

        for (auto &bp : program.bplist)
        {
            // unsigned long int bp_addr = program.base_address + bp.addr;

            byte ovalue = change_byte(program.base_address + bp.addr, 0xcc);
            bp.ovalue = ovalue;
        }
        printf("** pid %d\n", program.pid);
    }
    return 0;
}