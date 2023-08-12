#pragma once

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <mswsock.h>


namespace RzLib
{
    constexpr size_t MAX_TCP_PACKAGE_SIZE = 1500;

    class Server
    {
    public:
        Server(std::string&& ip, int port);
        ~Server();
        bool Init();
        bool Listen();
        bool Accept();

        void ListClient();
        int GetPort(SOCKET socket);

        bool SendFileToClient(SOCKET socket, const std::string& path);
    private:
        void HandleServerCMD();
        void HandleClientCMD(SOCKET socket, const char* CMD);

        void AcceptClient();
        bool GetClientMsg(SOCKET socket, char* buf);

    private:
        std::string m_ip;
        int m_port;
        SOCKET m_listen_socket;
        std::vector<std::pair<SOCKET, int>> m_client;
        fd_set m_All_FD;
    };
}