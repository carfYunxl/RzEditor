#include "RzClient/RzClient.hpp"
#include "RzCore/Log.hpp"
#include "RzUtility/Utility.hpp"
#include "RzFile/FileTraveler.hpp"
#include "RzThread/RzThread.hpp"
#include <fstream>

int main()
{
    // must add this to eable color out to console
    RzLib::Utility::EnSTDOutputColor();
    RzLib::Utility::ChangeConsoleFont(L"Consolas");

    //检查当前目录下是否含有_old的exe和dll档案，有的话就直接删除
    {
        RzLib::ScopeThread thread(std::thread([]() {
            RzLib::Log(RzLib::LogLevel::ERR, "delete old exe and dll file ......");
            for (const auto dir_entry : std::filesystem::recursive_directory_iterator{ std::filesystem::current_path() })
            {
                std::string filename = dir_entry.path().filename().string();

                if (filename.find("_old") != std::string::npos)
                {
                    RzLib::Log(RzLib::LogLevel::ERR, "try delete file ......", dir_entry.path().string());

                    while (1)
                    {
                        if (!std::filesystem::remove(dir_entry.path()))
                        {
                            RzLib::Log(RzLib::LogLevel::ERR, "delete file failure ......error code = ", GetLastError());
                        }
                        else
                        {
                            RzLib::Log(RzLib::LogLevel::ERR, "delete file success ......", dir_entry.path().string());
                            break;
                        }
                    }
                }
            }
        }));
    }

    RzLib::RzClient client("127.0.0.1", 8080);

    client.LoadIni();

    printf("\n//===========  当前客户端版本 =================//\n");
    printf("RzLibrary Client : %s\n", client.GetClientVer().c_str());
    printf("//============================================//\n");

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
