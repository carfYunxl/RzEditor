#include "RzClient/RzClient.hpp"
#include "RzCore/Log.hpp"
#include "RzUtility/Utility.hpp"
#include "RzFile/FileTraveler.hpp"
#include <fstream>

int main()
{
    //��ȡ�����ļ��еİ汾��Ϣ
    std::filesystem::path p = std::filesystem::current_path() / "Client.ini";

    std::string str;
    if (std::filesystem::exists(p))
    {
        std::ifstream inf(p.string(), std::ifstream::in);

        if (!inf.is_open())
        {
            return -1;
        }
        inf.seekg(0, std::ios::end);
        size_t size = inf.tellg();
        str.resize(size);
        if (size != -1)
        {
            inf.seekg(0, std::ios::beg);
            inf.read(&str[0], size);
            inf.close();
        }

        int index = str.find("[ver]");
        str = str.substr(index + 6, 4);

    }

    printf("//===========  ��ǰ�ͻ��˰汾 =================//\n");
    printf("RzLibrary Client : %s\n", str.c_str());
    printf("//============================================//\n");

    //��鵱ǰĿ¼���Ƿ���_old��exe��dll�������еĻ���ֱ��ɾ��
    for (const auto dir_entry : std::filesystem::recursive_directory_iterator{ std::filesystem::current_path() })
    {
        std::string filename = dir_entry.path().string();

        if (filename.find("_old") != std::string::npos)
        {
            std::filesystem::remove(dir_entry.path());
        }
    }

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
