#pragma once
#include <vector>
#include <string>
#include <winsock2.h>
#include <filesystem>
#include "RzCMD/ConsoleCMDParser.hpp"
#include "RzSlim.h"

namespace RzLib
{
    constexpr size_t MAX_TCP_PACKAGE_SIZE = 1450;

    constexpr short CLIENT_VERSION = 0x1021;        // 客户端版本协定 v1.0.0.9

    static const std::string QUIT = "q";

    using vecClient = std::vector<std::pair<SOCKET, int>>;


    enum class InputMode
    {
        CONSOLE = 0,    // wait for console server cmd 
        SELECT,         // select some choice
        SEND            // input message to send to someone
    };

    class CMD;

    class RzServer
    {
    public:
        RzServer(RzSlim* UI, std::string&& ip, int port);
        ~RzServer();

        void Start();
        void StopServer() { m_IsRunning = false; }
        void ListClient();
        int  GetPort(SOCKET socket);
        bool IsClientSocket(size_t nSocket);

        void SetInputMode(InputMode mode) { m_Mode = mode; }
        void SelectClient(SOCKET socket) { m_client_socket = socket; }

        void SendInfo( TCP_CMD cmd, const std::string& msg);

        const std::filesystem::path GetCurrentDir() const { return m_DirPath; }
        void SetCurrentDir(const std::filesystem::path path) { m_DirPath = path; }

        RzSlim* GetUI() { return m_UI; }

    private:
        bool Init();
        bool Listen();
        bool Accept();
        void AcceptInput();
        void AcceptClient(SOCKET socket, const char* CMD, int rtLen);
        void AcceptConnection();
        bool GetClientMsg(SOCKET socket, char* buf, int* rtlen);

        void PrintConsoleHeader(const std::string& path);

        void AcceptRequest();
    private:
        RzSlim*         m_UI;
        std::string     m_ip;
        int             m_port;
        SOCKET          m_listen_socket;
        vecClient       m_client;
        fd_set          m_All_FD;
        bool            m_IsRunning;
        InputMode       m_Mode;
        SOCKET          m_client_socket;

        std::filesystem::path m_DirPath;
    };
}