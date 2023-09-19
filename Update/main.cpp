#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>
#include <stdio.h>
#include "OverLappedIO/OlpIO.hpp"

int main()
{
    RzLib::OverLappedIO_AcceptEx* io = new RzLib::OverLappedIO_AcceptEx;

    if (!io->Init())
        return -1;

    if (!io->Run())
    {
        return -1;
    }

    delete io;
    return 0;
}
