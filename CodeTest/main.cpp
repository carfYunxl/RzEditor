#include <iostream>
#include "Net/WinServerExample/HighPerformanceServer.h"

int __cdecl main()
{
    RzLib::HighPerformanceServer server;

    try
    {
        server.Init();

        server.Run();
    }
    catch (const std::exception& exception)
    {
        std::cout << exception.what() << std::endl;
    }

    std::cin.get();
    return 0;
}