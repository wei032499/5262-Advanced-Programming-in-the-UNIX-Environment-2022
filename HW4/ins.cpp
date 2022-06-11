#include <sstream>
#include <vector>
#include <list>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <string.h>

#include "util.h"
#include "ins.h"
#include "load.h"
#include "start.h"
#include "disasm.h"

using namespace std;

int (*func[16])(vector<string> &);

void execins(string line)
{

    init_func();
    stringstream ss(line);
    string ins, arg;
    vector<string> args;
    ss >> ins;
    while (ss >> arg)
        args.push_back(arg);

    int funcid = -1;

    ins == "break" || ins == "b"    ? funcid = 0
    : ins == "cont" || ins == "c"   ? funcid = 1
    : ins == "delete"               ? funcid = 2
    : ins == "disasm" || ins == "d" ? funcid = 3
    : ins == "dump" || ins == "x"   ? funcid = 4
    : ins == "exit" || ins == "q"   ? funcid = 5
    : ins == "get" || ins == "g"    ? funcid = 6
    : ins == "getregs"              ? funcid = 7
    : ins == "help" || ins == "h"   ? funcid = 8
    : ins == "list" || ins == "l"   ? funcid = 9
    : ins == "load"                 ? funcid = 10
    : ins == "run" || ins == "r"    ? funcid = 11
    : ins == "vmmap" || ins == "m"  ? funcid = 12
    : ins == "set" || ins == "s"    ? funcid = 13
    : ins == "si"                   ? funcid = 14
    : ins == "start"                ? funcid = 15
                                    : -1;

    if (funcid == -1)
        fprintf(stderr, "%s\n", ("** known instruction: " + ins).c_str());
    else
        func[funcid](args);
}

int setbreak(unsigned long long addr)
{

    /*unordered_map<unsigned long long, vector<struct bp>::const_iterator>::const_iterator got = program.bpmap.find(addr);

if (got == program.bpmap.end())
{
    program.bplist.push_back(point);
    program.bpmap[addr] = program.bplist.end();
}
else
    return -1;*/

    for (auto &bp : program.bplist)
    {
        if (addr == bp.addr)
            return -1;
    }
    struct bp point;
    point.addr = addr;
    point.ovalue = change_byte(program.base_address + addr, 0xcc);
    program.bplist.push_back(point);

    return 0;
}

int insbreak(vector<string> &args)
{
    if (args.size() != 1)
    {
        fprintf(stderr, "** Instruction Error: break\n");
        return -1;
    }
    else if (program.state != RUNNING)
    {
        fprintf(stderr, "** program is NOT running.\n");
        return -1;
    }
    unsigned long long addr = stoul(args[0], nullptr, 16);

    setbreak(addr);

    return 0;
}

int checkbp() // return bp idx, or -1 if not bp
{

    if (ptrace(PTRACE_GETREGS, program.pid, 0, &program.regs) != 0)
    {
        errquit("** ptrace error ");
        return -1;
    }

    byte data[sizeof(long)];
    *data = ptrace(PTRACE_PEEKDATA, program.pid, program.regs.rip - 1, NULL);

    for (auto &bp : program.bplist)
        change_byte(program.base_address + bp.addr, 0xcc);

    // printf("%llx %llx\n", program.regs.rip - 1 - program.base_address, data[0]);

    int bpid = 0;
    for (auto &bp : program.bplist)
    {
        if (program.regs.rip - 1 == program.base_address + bp.addr)
            return data[0] == 0xcc ? bpid : -1;
        bpid++;
    }
    // unordered_map<unsigned long long, vector<struct bp>::const_iterator>::const_iterator got = program.bpmap.find(program.regs.rip - 1 - program.base_address);
    // if (got != program.bpmap.end())
    //     return got->second - program.bplist.begin();
    return -1;
}

int restore(int bpid)
{
    if (ptrace(PTRACE_GETREGS, program.pid, 0, &program.regs) != 0)
    {
        fprintf(stderr, "** ptrace error: GETREGS");
        return -1;
    }

    /* restore break point */
    change_byte(program.base_address + program.bplist[bpid].addr, program.bplist[bpid].ovalue);

    // ptrace(PTRACE_SINGLESTEP, program.pid, NULL, NULL);

    // if (waitpid(program.pid, NULL, 0) >= 0)
    //     change_byte(program.base_address + program.bplist[bpid].addr, 0xcc);

    /* set registers */
    program.regs.rip = program.regs.rip - 1;
    if (ptrace(PTRACE_SETREGS, program.pid, 0, &program.regs) != 0)
    {
        fprintf(stderr, "** ptrace(SETREGS)");
        return -1;
    }

    return 0;
}
int inscont(vector<string> &args)
{
    if (args.size() != 0)
    {
        fprintf(stderr, "** Instruction Error: cont\n");
    }
    else if (program.state != RUNNING)
    {
        fprintf(stderr, "** program is NOT running.\n");
        return -1;
    }
    ptrace(PTRACE_SINGLESTEP, program.pid, NULL, NULL);

    if (waitpid(program.pid, NULL, 0) < 0)
        errquit("** cont step error ");

    int bpid = checkbp();
    if (bpid == -1)
    {
        ptrace(PTRACE_CONT, program.pid, NULL, NULL);

        int status;

        if (waitpid(program.pid, &status, 0) < 0)
        {
            perror("** inscont error ");
        }
        else if (WIFEXITED(status))
        {
            if (WIFSIGNALED(status))
                printf("** %d stop by signal", program.pid);
            else
                printf("** chlid process %d terminiated normally (code %d)\n", program.pid, status);
            program.state = LOADED;
            return 0;
        }
    }

    if ((bpid = checkbp()) != -1)
    {
        restore(bpid);

        printf("** breakpoint @");
        int count = capstone_disasm(program.bplist[bpid].addr, program.text.size - (program.bplist[bpid].addr - program.text.entry), 1);
    }

    return 0;
}
int insdelete(vector<string> &args)
{
    if (args.size() != 1)
    {
        fprintf(stderr, "** Instruction Error: delete\n");
        return -1;
    }
    else if (program.state != RUNNING)
    {
        fprintf(stderr, "** program is NOT running.\n");
        return -1;
    }

    int bpid = stoi(args[0]);

    if (bpid >= (int)program.bplist.size())
        return -1;

    change_byte(program.base_address + program.bplist[bpid].addr, program.bplist[bpid].ovalue);
    program.bplist.erase(program.bplist.begin() + bpid);
    printf("** breakpoint %d deleted\n", bpid);
    return 0;
}
int insdisasm(vector<string> &args)
{
    if (args.size() < 1)
    {
        fprintf(stderr, "** no addr is given.\n");
        return -1;
    }
    else if (args.size() != 1)
    {
        fprintf(stderr, "** disasm error\n");
        return -1;
    }
    else if (program.state != RUNNING)
    {
        fprintf(stderr, "** program is NOT running.\n");
        return -1;
    }

    unsigned long int addr;
    sscanf(args[0].data(), "0x%lx", &addr);

    if (addr < program.text.entry || program.text.entry + program.text.size < addr)
    {
        fprintf(stderr, "** the address is out of the range of the text segment.\n");
        return -1;
    }

    int count = capstone_disasm(addr, program.text.size - (addr - program.text.entry), 10); // program.text.entry + program.text.size - addr

    if (count < 10)
    {
        fprintf(stderr, "** the address is out of the range of the text segment.\n");
        return -1;
    }

    return 0;
}
int insdump(vector<string> &args)
{
    if (args.size() != 1)
    {
        fprintf(stderr, "** dump error\n");
        return -1;
    }
    else if (program.state != RUNNING)
    {
        fprintf(stderr, "** program is NOT running.\n");
        return -1;
    }
    else if (args.size() < 1)
    {
        fprintf(stderr, "** no addr is given.\n");
        return -1;
    }

    unsigned long long addr;
    sscanf(args[0].data(), "0x%llx", &addr);
    // byte data[80];
    // long *ptr = (long *)data;
    // for (int i = 0; i < 80 / sizeof(long); i++)
    // {
    //     *ptr = ptrace(PTRACE_PEEKDATA, program.pid, program.base_address + addr + i * sizeof(long), NULL);
    //     ptr += 1;
    // }
    // for (int i = 0; i < 80; i++)
    //     printf("%02x ", data[i]);
    byte data[16];
    long *ptr;
    for (int i = 0; i < 5; i++)
    {
        ptr = (long *)data;
        printf("      %06llx: ", addr);
        for (unsigned int j = 0; j < 16 / sizeof(long); j++)
        {
            *ptr = ptrace(PTRACE_PEEKDATA, program.pid, program.base_address + addr + j * sizeof(long), NULL);
            ptr += 1;
        }
        addr += 16;

        for (int j = 0; j < 16; j++)
            printf("%02x ", data[j]);
        printf(" |");
        for (int j = 0; j < 16; j++)
            isprint(data[j]) ? printf("%c", data[j]) : printf(".");
        printf("|\n");
    }

    return 0;
}
int insexit(vector<string> &args)
{
    if (args.size() != 0)
    {
        fprintf(stderr, "** Instruction Error: exit\n");
        return -1;
    }

    exit(0);

    return 0;
}
int insget(vector<string> &args)
{
    if (args.size() != 1)
    {
        fprintf(stderr, "** Instruction Error: get\n");
        return -1;
    }
    else if (program.state != RUNNING)
    {
        fprintf(stderr, "** program is NOT running.\n");
        return -1;
    }
    else if (ptrace(PTRACE_GETREGS, program.pid, 0, &program.regs) != 0)
    {
        errquit("** ptrace error ");
        return -1;
    }

    if (args[0] == "rax")
        printf("rax = %llu (0x%llx)\n", program.regs.rax, program.regs.rax);
    else if (args[0] == "rbx")
        printf("rbx = %llu (0x%llx)\n", program.regs.rbx, program.regs.rbx);
    else if (args[0] == "rcx")
        printf("rcx = %llu (0x%llx)\n", program.regs.rcx, program.regs.rcx);
    else if (args[0] == "rdx")
        printf("rdx = %llu (0x%llx)\n", program.regs.rdx, program.regs.rdx);
    else if (args[0] == "r8")
        printf("r8 = %llu (0x%llx)\n", program.regs.r8, program.regs.r8);
    else if (args[0] == "r9")
        printf("r9 = %llu (0x%llx)\n", program.regs.r9, program.regs.r9);
    else if (args[0] == "r10")
        printf("r10 = %llu (0x%llx)\n", program.regs.r10, program.regs.r10);
    else if (args[0] == "r11")
        printf("r11 = %llu (0x%llx)\n", program.regs.r11, program.regs.r11);
    else if (args[0] == "r12")
        printf("r12 = %llu (0x%llx)\n", program.regs.r12, program.regs.r12);
    else if (args[0] == "r13")
        printf("r13 = %llu (0x%llx)\n", program.regs.r13, program.regs.r13);
    else if (args[0] == "r14")
        printf("r14 = %llu (0x%llx)\n", program.regs.r14, program.regs.r14);
    else if (args[0] == "r15")
        printf("r15 = %llu (0x%llx)\n", program.regs.r15, program.regs.r15);
    else if (args[0] == "rdi")
        printf("rdi = %llu (0x%llx)\n", program.regs.rdi, program.regs.rdi);
    else if (args[0] == "rsi")
        printf("rsi = %llu (0x%llx)\n", program.regs.rsi, program.regs.rsi);
    else if (args[0] == "rbp")
        printf("rbp = %llu (0x%llx)\n", program.regs.rbp, program.regs.rbp);
    else if (args[0] == "rsp")
        printf("rsp = %llu (0x%llx)\n", program.regs.rsp, program.regs.rsp);
    else if (args[0] == "rip")
        printf("rip = %llu (0x%llx)\n", program.regs.rip, program.regs.rip);
    else if (args[0] == "flags")
        printf("flags = %llu (0x%llx)\n", program.regs.eflags, program.regs.eflags);
    else
    {
        fprintf(stderr, "** known register\n");
        return -1;
    }

    return 0;
}
int insgetregs(vector<string> &args)
{
    if (args.size() != 0)
    {
        fprintf(stderr, "** Instruction Error: getregs\n");
        return -1;
    }
    else if (program.state != RUNNING)
    {
        fprintf(stderr, "** program is NOT running.\n");
        return -1;
    }

    if (ptrace(PTRACE_GETREGS, program.pid, 0, &program.regs) != 0)
    {
        fprintf(stderr, "** ptrace error: GETREGS");
        return -1;
    }

    printf("RAX %-14llx RBX %-14llx RCX %-14llx RDX %-14llx\n", program.regs.rax, program.regs.rbx, program.regs.rcx, program.regs.rdx);
    printf("R8  %-14llx R9  %-14llx R10 %-14llx R11 %-14llx\n", program.regs.r8, program.regs.r9, program.regs.r10, program.regs.r11);
    printf("R12 %-14llx R13 %-14llx R14 %-14llx R15 %-14llx\n", program.regs.r12, program.regs.r13, program.regs.r14, program.regs.r15);
    printf("RDI %-14llx RSI %-14llx RBP %-14llx RSP %-14llx\n", program.regs.rdi, program.regs.rsi, program.regs.rbp, program.regs.rsp);
    printf("RIP %-14llx FLAGS %016llx\n", program.regs.rip, program.regs.eflags);

    return 0;
}
int inshelp(vector<string> &args)
{
    if (args.size() != 0)
    {
        fprintf(stderr, "** Instruction Error: help\n");
        return -1;
    }

    printf("- break {instruction-address}: add a break point\n");
    printf("- cont: continue execution\n");
    printf("- delete {break-point-id}: remove a break point\n");
    printf("- disasm addr: disassemble instructions in a file or a memory region\n");
    printf("- dump addr: dump memory content\n");
    printf("- exit: terminate the debugger\n");
    printf("- get reg: get a single value from a register\n");
    printf("- getregs: show registers\n");
    printf("- help: show this message\n");
    printf("- list: list break points\n");
    printf("- load {path/to/a/program}: load a program\n");
    printf("- run: run the program\n");
    printf("- vmmap: show memory layout\n");
    printf("- set reg val: get a single value to a register\n");
    printf("- si: step into instruction\n");
    printf("- start: start the program and stop at the first instruction\n");

    return 0;
}
int inslist(vector<string> &args)
{
    if (args.size() != 0)
        fprintf(stderr, "** Instruction Error: list\n");

    int bpid = 0;
    for (auto &bp : program.bplist)
        printf("  %d:%8llx\n", bpid++, bp.addr);

    return 0;
}
int insload(vector<string> &args)
{
    if (args.size() != 1)
    {
        fprintf(stderr, "** Instruction Error: load\n");
        return -1;
    }
    else if (program.state >= LOADED)
    {
        fprintf(stderr, "** program is loaded.\n");
        return -1;
    }

    char *argv[args.size() + 1] = {NULL};
    for (unsigned int i = 0; i < args.size(); i++)
    {
        char *arg = new char[args[i].length() + 1]();
        copy(args[i].begin(), args[i].end(), arg);
    }
    program.path = args[0];
    program.argv = argv;
    load();

    return 0;
}
int insrun(vector<string> &args)
{
    if (args.size() != 0)
    {
        fprintf(stderr, "** Instruction Error: run\n");
        return -1;
    }

    if (program.state == RUNNING)
    {
        fprintf(stderr, "** program %s is already running\n", program.path.c_str());
        inscont(args);
        return 0;
    }
    else if (program.state == LOADED)
    {
        start();
        inscont(args);
        return 0;
    }

    fprintf(stderr, "** program is NOT loaded yet.\n");
    return -1;

    return 0;
}
int insvmmap(vector<string> &args)
{
    if (args.size() != 0)
    {
        fprintf(stderr, "** Instruction Error: vmmap\n");
        return -1;
    }
    else if (program.state != RUNNING)
    {
        fprintf(stderr, "** program is NOT running.\n");
        return -1;
    }

    char path[4097];
    snprintf(path, 4097, "/proc/%d/maps", program.pid);
    FILE *f = fopen(path, "r");
    if (f == NULL)
    {
        perror("** fopen error: ");
        return -1;
    }

    unsigned long long addr, endaddr, offset, inode;
    char premission[4097], device[4097], filename[4097];

    while (true)
    {
        int ret = fscanf(f, "%llx-%llx %s %llx %s %llx", &addr, &endaddr, premission, &offset, device, &inode);
        if (!(ret != 0 && ret != EOF))
            break;
        fscanf(f, "%s", filename);
        fgets(filename + strlen(filename), 4097, f);
        if (filename[strlen(filename) - 1] == '\n')
            filename[strlen(filename) - 1] = '\0';

        /*if (ret > 0 && ret != EOF && inode != 0)
        {


            // ret += fscanf(f, "%s\n", filename);
            // if (!(ret != 0 && ret != EOF))
            //     break;
        }
        else
        {
            char buf[4097];
            filename[0] = '\0';
            fgets(buf, 4097, f);
            sscanf(buf, "%s\n", filename);
        }*/
        printf("%016llx-%016llx %c%c%c %llu        %s\n", addr, endaddr, premission[0], premission[1], premission[2], offset, filename);
    }

    fclose(f);

    return 0;
}
int insset(vector<string> &args)
{
    if (args.size() != 2)
    {
        fprintf(stderr, "** Instruction Error: set\n");
        return -1;
    }
    else if (program.state != RUNNING)
    {
        fprintf(stderr, "** program is NOT running.\n");
        return -1;
    }
    else if (ptrace(PTRACE_GETREGS, program.pid, 0, &program.regs) != 0)
    {
        errquit("** ptrace error ");
        return -1;
    }

    unsigned long int addr;
    sscanf(args[1].data(), "0x%lx", &addr);

    if (args[0] == "rax")

        program.regs.rax = addr;
    else if (args[0] == "rbx")
        program.regs.rbx = addr;
    else if (args[0] == "rcx")
        program.regs.rcx = addr;
    else if (args[0] == "rdx")
        program.regs.rdx = addr;
    else if (args[0] == "r8")
        program.regs.r8 = addr;
    else if (args[0] == "r9")
        program.regs.r9 = addr;
    else if (args[0] == "r10")
        program.regs.r10 = addr;
    else if (args[0] == "r11")
        program.regs.r11 = addr;
    else if (args[0] == "r12")
        program.regs.r12 = addr;
    else if (args[0] == "r13")
        program.regs.r13 = addr;
    else if (args[0] == "r14")
        program.regs.r14 = addr;
    else if (args[0] == "r15")
        program.regs.r15 = addr;
    else if (args[0] == "rdi")
        program.regs.rdi = addr;
    else if (args[0] == "rsi")
        program.regs.rsi = addr;
    else if (args[0] == "rbp")
        program.regs.rbp = addr;
    else if (args[0] == "rsp")
        program.regs.rsp = addr;
    else if (args[0] == "rip")
        program.regs.rip = addr;
    else if (args[0] == "flags")
        program.regs.eflags = addr;
    else
    {
        fprintf(stderr, "** known register\n");
        return -1;
    }

    if (ptrace(PTRACE_SETREGS, program.pid, 0, &program.regs) != 0)
    {
        fprintf(stderr, "** ptrace(SETREGS)");
        return -1;
    }

    return 0;
}
int inssi(vector<string> &args)
{
    if (args.size() != 0)
        fprintf(stderr, "** Instruction Error: si\n");

    ptrace(PTRACE_SINGLESTEP, program.pid, NULL, NULL);

    if (waitpid(program.pid, NULL, 0) < 0)
        errquit("** si error ");

    int bpid = checkbp();
    if (bpid != -1)
    {
        restore(bpid);

        printf("** breakpoint @");
        int count = capstone_disasm(program.bplist[bpid].addr, program.text.size - (program.bplist[bpid].addr - program.text.entry), 1);
    }

    // ptrace(PTRACE_SINGLESTEP, program.pid, NULL, NULL);

    // if (waitpid(program.pid, NULL, 0) >= 0 && (bpid = checkbp()) != -1)
    // {
    //     printf("** breakpoint @");

    //     int count = capstone_disasm(program.bplist[bpid].addr, program.text.size - (program.bplist[bpid].addr - program.text.entry), 1);
    // }

    return 0;
}
int insstart(vector<string> &args)
{
    if (args.size() != 0)
        fprintf(stderr, "** Instruction Error: start\n");

    start();
    return 0;
}

void init_func()
{
    static bool inited = false;
    if (inited)
        return;

    func[0] = insbreak;
    func[1] = inscont;
    func[2] = insdelete;
    func[3] = insdisasm;
    func[4] = insdump;
    func[5] = insexit;
    func[6] = insget;
    func[7] = insgetregs;
    func[8] = inshelp;
    func[9] = inslist;
    func[10] = insload;
    func[11] = insrun;
    func[12] = insvmmap;
    func[13] = insset;
    func[14] = inssi;
    func[15] = insstart;

    inited = true;
}
