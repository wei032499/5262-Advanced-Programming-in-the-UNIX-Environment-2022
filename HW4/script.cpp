#include <stdio.h>
#include <fstream>

#include "ins.h"
#include "util.h"

using namespace std;
/*void execscript(char *script)
{
    fstream file(script, ios::in);
    if (file.fail())
        errquit("Script file error: ");

    string line;

    while (getline(file, line))
        execins(line);

    file.close();
}
*/