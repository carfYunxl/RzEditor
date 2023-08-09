#include  "Log.hpp"
#include "Server/Server.hpp"

int main()
{
    RzLib::Server server("192.168.2.9",8080);

    if (!server.Init())
    {
        RzLib::Log(RzLib::LogLevel::ERR,"server init error, error code : ", WSAGetLastError());
        return 0;
    }

    if (!server.Listen())
    {
        RzLib::Log(RzLib::LogLevel::ERR, "server listen error, error code : ", WSAGetLastError());
        return 0;
    }

    if (!server.Accept())
    {
        RzLib::Log(RzLib::LogLevel::ERR, "server accept error, error code : ", WSAGetLastError());
        return 0;
    }

    return 0;
}
