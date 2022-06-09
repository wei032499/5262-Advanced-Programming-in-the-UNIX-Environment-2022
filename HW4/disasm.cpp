
#include <capstone/capstone.h>
#include "disasm.h"

int capstone_disasm(unsigned long int start, unsigned long int size, int num)
{
    csh handle;
    cs_insn *insn;
    size_t count;

    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK)
        return -1;

    count = cs_disasm(handle, program.text.data + (start - program.text.entry), size, start, num, &insn);
    if (count > 0)
    {
        size_t j;
        for (j = 0; j < count; j++)
        {
            printf("      %lx: ", insn[j].address);
            for (int k = 0; k < 10; k++)
                k < insn[j].size ? printf("%02x ", insn[j].bytes[k]) : printf("   ");

            for (int k = 10; k < insn[j].size; k++)
                k < insn[j].size ? printf("%02x ", insn[j].bytes[k]) : printf("   ");

            printf(" %s       %s\n", insn[j].mnemonic, insn[j].op_str);
        }

        cs_free(insn, count);
    }
    else
        printf("** ERROR: Failed to disassemble given code!\n");

    cs_close(&handle);

    return count;
}