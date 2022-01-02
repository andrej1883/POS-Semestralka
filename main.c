#include "server.h"
#include "client.h"
#include "string.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
    char* newArgv[argc - 1];
    newArgv[0] = argv[0];
    for (int i = 2; i < argc; ++i)
    {
        newArgv[i - 1] = argv[i];
    }

    if (strcmp(argv[1], "server") == 0)
    {
        return server(argc - 1, newArgv);
    }
    else if(strcmp(argv[1], "client") == 0)
    {
        return client(argc - 1, newArgv);
    }
    else
    {
        fprintf(stderr, "wrong arguments\n");
        return 1;
    }
}

