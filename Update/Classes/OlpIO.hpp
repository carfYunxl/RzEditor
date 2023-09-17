#pragma once
#include <winsock2.h>
#include <windows.h>
#include <vector>
#include <mutex>

namespace RzLib
{
    constexpr size_t PORT = 8080;

    constexpr size_t DATA_BUFSIZE = 4096;

    class Server
    {
    public:
        void init();
        void install();
        bool start();
        bool stop();
    private:

    };

    struct SocketUnit
    {
        CHAR Buffer[DATA_BUFSIZE];
        WSABUF DataBuf;
        SOCKET Socket;
        WSAOVERLAPPED Overlapped;
        DWORD BytesSEND;
        DWORD BytesRECV;
    };

    class OverLappedIO
    {
    public:
        OverLappedIO() = default;
        ~OverLappedIO() { WSACleanup(); }

        bool Init();
        bool Run();

    private:
        void ProcessIO();

    private:
        SOCKET                  m_ListenSocket;
        std::vector<WSAEVENT>   m_EventVec;
        std::mutex              m_mutex;
        std::vector<SocketUnit*> m_SocketUnitVec;
    };
}
