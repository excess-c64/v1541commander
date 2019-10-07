#include "v1541commander.h"

int main(int argc, char **argv)
{
    V1541Commander commander(argc, argv);
    return commander.exec();
}

