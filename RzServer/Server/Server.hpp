#pragma once

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <mswsock.h>


namespace RzLib
{
    class Server
    {
    public:
        Server(std::string&& ip, int port);
        ~Server();
        bool Init();
        bool Listen();
        bool Accept();
    private:
        std::string m_ip;
        int m_port;
        SOCKET m_listen_socket;
        std::vector<std::pair<SOCKET, int>> client;
        fd_set m_All_FD;
    };
}