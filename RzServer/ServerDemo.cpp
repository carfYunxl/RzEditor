#include "Core/Log.hpp"
#include "TcpServer/RzServer.hpp"

int main()
{
    // must add this to eable color out to console

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    RzLib::RzServer server("127.0.0.1",8080);

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
