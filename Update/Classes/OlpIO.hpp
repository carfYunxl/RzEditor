#pragma once
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

namespace RzLib
{
    class Server
    {
    public:
        void init();
        void install();
        bool start();
        bool stop();
    private:

    };

    class OverLappedIO
    {

    };
}
