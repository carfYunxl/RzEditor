#include "RzClient/RzClient.hpp"
#include "RzCore/Log.hpp"
#include "RzUtility/Utility.hpp"

int main()
{
    // must add this to eable color out to console
    RzLib::Utility::EnSTDOutputColor();
    RzLib::Utility::ChangeConsoleFont(L"Consolas");

    RzLib::RzClient client("127.0.0.1", 8080);

    if (!client.Init())
    {
        RzLib::Log(RzLib::LogLevel::ERR, "client init error, error code : ", WSAGetLastError());
        return 0;
    }

    if (!client.Connect())
    {
        RzLib::Log(RzLib::LogLevel::ERR, "client connect error, error code : ", WSAGetLastError());
        return 0;
    }

    return 0;
}
