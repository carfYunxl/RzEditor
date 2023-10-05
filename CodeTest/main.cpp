#include <iostream>
#include "Net/WinServerExample/HighPerformanceServer.h"

int __cdecl mainxx()
{
    RzLib::HighPerformanceServer server;
    server.Init();

    std::cin.get();
    return 0;
}