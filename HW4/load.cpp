#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>
// #include <algorithm>
#include <bitset>

#include "util.h"
#include "load.h"

// using namespace std;

int parse_elf(const char *path)
{

    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return -1;

    Elf64_Ehdr header;
    read(fd, &header, sizeof(Elf64_Ehdr));

    // program.load.entry = header.e_entry;
    program.dynamic = header.e_type == ET_DYN;

    Elf64_Shdr shtable[header.e_shnum];
    lseek(fd, header.e_shoff, SEEK_SET);
    read(fd, shtable, sizeof(Elf64_Shdr) * header.e_shnum);

    Elf64_Shdr *sh_strtab = &shtable[header.e_shstrndx];

    char strsec[sh_strtab->sh_size];
    lseek(fd, sh_strtab->sh_offset, SEEK_SET);
    read(fd, strsec, sh_strtab->sh_size);

    for (int i = 0; i < header.e_shnum; ++i)
    {
        if (strcmp(strsec + shtable[i].sh_name, ".text") == 0)
        {
            // program.text.offset = shtable[i].sh_offset;
            program.text.size = shtable[i].sh_size;
            program.text.entry = shtable[i].sh_addr;
            program.text.data = (byte *)malloc(shtable[i].sh_size);
            lseek(fd, shtable[i].sh_offset, SEEK_SET);
            read(fd, program.text.data, shtable[i].sh_size);

            return 0;
        }
    }

    return -1;
}

void load()
{
    if (program.state > State::NONE)
    {
        fprintf(stderr, "** Program had been loaded! ");
        return;
    }

    if (parse_elf(program.path.c_str()) < 0)
        perror("** Load error ");
    else
    {
        program.state = State::LOADED;
        printf("** program '%s' loaded. entry point 0x%lx\n", program.path.c_str(), program.text.entry);
    }
}