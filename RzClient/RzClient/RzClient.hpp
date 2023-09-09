#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <thread>
#include <string>

#include <filesystem>

#include "RzCore/Core.hpp"

namespace RzLib
{
    constexpr size_t MAX_TCP_PACKAGE_SIZE = 1450;

    struct IniInfo
    {
        std::string client_ver;
    };

    class RzClient
    {
    public:
        RzClient(const std::string& server_ip, uint32_t server_port);
        RzClient(std::string&& server_ip, uint32_t&& server_port);

        ~RzClient();

        void Run();

        const SOCKET GetSocket() const { return m_socket; }

        bool Recv();
        bool Send();

        bool UpdateClient();

        const std::string GetClientVer() const { return m_iniInfo.client_ver; }

        void LoadIni();
        void SaveIni();

    private:
        bool Init();
        bool Connect();
        void CreateDir(const std::string& dirName);
        void Update(const std::filesystem::path& path);
        void StopClient() { m_Running = false; }

        std::pair<TCP_CMD, std::string> ReadPacket();
    private:
        std::string             m_serverIp;
        uint32_t                m_serverPort;
        SOCKET                  m_socket;

        std::filesystem::path   m_pCurPath;
        std::filesystem::path   m_pRootPath;    //exeËùÔÚÄ¿Â¼
        std::string             m_fCurContent;
        bool                    m_Running{ true };
        IniInfo                 m_iniInfo;

        std::filesystem::path   m_clientPath;
    };
}
