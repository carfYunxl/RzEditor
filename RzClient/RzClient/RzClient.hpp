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

    const std::string CLIENT_VERSION = "1001";

    class RzClient
    {
    public:
        RzClient(const std::string& server_ip, uint32_t server_port);
        RzClient(std::string&& server_ip, uint32_t&& server_port);

        ~RzClient();

        const SOCKET GetSocket() const { return m_socket; }
        const std::string GetVersion() const { return m_Version; }

        bool Recv();
        bool Send();

        bool Init();
        bool Connect();

        bool UpdateClient();

        void Update(bool update) { m_updated = update; }
        bool IsUpdating() { return m_updated; }

        const std::string GetClientVer() const { return m_new_client_ver; }

    private:
        void CreateDir(const std::string& dirName);
        void Update(const std::filesystem::path& path);
        void StopClient() { m_Running = false; }

        std::pair<TCP_CMD, std::string> ReadPacket();
    private:
        std::string             m_serverIp;
        uint32_t                m_serverPort;
        SOCKET                  m_socket;

        std::string             m_Version;
        bool                    m_updated;

        std::filesystem::path   m_pCurPath;
        std::filesystem::path   m_pRootPath;    //exeËùÔÚÄ¿Â¼
        std::string             m_fCurContent;
        bool                    m_Running{true};
        std::string             m_new_client_ver;
    };
}
