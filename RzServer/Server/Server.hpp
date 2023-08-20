#pragma once

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <mswsock.h>
#include <memory>

#include "CMD/ConsoleCMDParser.hpp"
#include "CMD/CMDType.hpp"


namespace RzLib
{
    constexpr size_t MAX_TCP_PACKAGE_SIZE = 1500;
    constexpr short CLIENT_VERSION = 0x1001;        // 客户端版本协定 v1.0.0.1

    using vecClient = std::vector<std::pair<SOCKET, int>>;

    class Server
    {
    public:
        Server(std::string&& ip, int port);
        ~Server();
        bool Init();
        bool Listen();
        bool Accept();

        void StopServer() { m_IsRunning = false; }

        void ListClient();
        int GetPort(SOCKET socket);

        bool SendFileToClient(SOCKET socket, const std::string& Cmd, const std::string& path);

        bool SendClientVersion(SOCKET socket);
    private:
        void HandleServerCMD();
        void HandleClientCMD(SOCKET socket, const char* CMD, int rtLen);

        void AcceptClient();
        bool GetClientMsg(SOCKET socket, char* buf, int* rtlen);

        std::unique_ptr<CMD> GenCmd(const ConsoleCMDParser& parser);

    private:
        std::string     m_ip;
        int             m_port;
        SOCKET          m_listen_socket;
        vecClient       m_client;
        fd_set          m_All_FD;
        bool            m_IsRunning;
    };
}