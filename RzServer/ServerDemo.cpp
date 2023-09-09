#include "RzCore/Log.hpp"
#include "RzServer/RzServer.hpp"
#include "RzUtility/Utility.hpp"

int main()
{
    // must add this to enable color out to console
    RzLib::Utility::EnSTDOutputColor();
    //RzLib::Utility::ChangeConsoleFont(L"Consolas");
    RzLib::Utility::ChangeConsoleFont(15, L"Consolas");

    RzLib::RzServer server("127.0.0.1",8080);

    server.Start();

    return 0;
}
