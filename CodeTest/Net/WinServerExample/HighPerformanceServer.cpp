#include "HighPerformanceServer.h"
#include "RzCore/Log.hpp"
#include "AddrInfoWrapper.h"
#include <thread>

namespace RzLib
{
    bool HighPerformanceServer::Init()
    {
        WSADATA sData;
        if (WSAStartup(MAKEWORD(2, 2), &sData) != 0)
        {
            Log(LogLevel::ERR, "fail to load Winsock!");
            return false;
        }

        HANDLE CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)NULL, 0);
        if (CompletionPort == NULL)
        {
            Log(LogLevel::ERR, "CreateIoCompletionPort failed : ", GetLastError());
            return false;
        }

        SYSTEM_INFO sInfo;
        GetSystemInfo(&sInfo);
        if (sInfo.dwNumberOfProcessors > MAX_COMPLETION_THREAD_COUNT)
        {
            sInfo.dwNumberOfProcessors = MAX_COMPLETION_THREAD_COUNT;
        }

        // Round the buffer size to the next increment of the page size
        if ((mBufferSize % sInfo.dwPageSize) != 0)
        {
            mBufferSize = ((mBufferSize / sInfo.dwPageSize) + 1) * sInfo.dwPageSize;
        }
        Log(LogLevel::INFO, "Buffer size = ", mBufferSize, "page size = ", sInfo.dwPageSize);

        for (int i = 0; i < (int)sInfo.dwNumberOfProcessors; i++)
        {
            std::thread th(&HighPerformanceServer::ProcessEvent, this, CompletionPort);
            HANDLE hHandle = static_cast<HANDLE>(th.native_handle());
            m_Events.push_back(hHandle);
            th.detach();
        }

        try 
        {
            AddrInfoWrapper wrapper;
        }
        catch (const std::exception& exception)
        {
            std::cout << exception.what() << std::endl;
            return false;
        }

        if (!wrapper.Init())
        {

        }
#if 0
        // 获取本地ip地址
        // 稍后会在这些ip地址上，监听所有可能的连接
        struct addrinfo hints, * res = NULL;
        memset(&hints, 0, sizeof(hints));
        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        int rc = getaddrinfo("", "", &hints, &res);

        while (res)
        {
            SOCKADDR_IN* addr = (SOCKADDR_IN*)res->ai_addr;

            char s[1024];
            std::cout << inet_ntop(addr->sin_family, &addr->sin_addr, s, 1024) << std::endl;
            std::cout << ntohl(addr->sin_port) << std::endl;

            // 给定IP地址，可以获取主机名
            char hostname[NI_MAXHOST];
            char server[NI_MAXHOST];
            DWORD dwRetVal = getnameinfo((SOCKADDR*)addr, sizeof(SOCKADDR) + 16, hostname, NI_MAXHOST, server, NI_MAXHOST, 0);

            if (dwRetVal == 0)
            {
                std::cout << hostname << std::endl;
                std::cout << server << std::endl;
            }


            res = res->ai_next;
        }

        freeaddrinfo(res);
#endif
    }

    void HighPerformanceServer::ProcessEvent(HANDLE nComPort)
    {
        //TO DO
    }
}
