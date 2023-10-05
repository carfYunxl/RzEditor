#include "AddrInfoWrapper.h"

namespace RzLib
{
    void AddrInfoWrapper::Initialized()
    {
        WSAData sData; 
        int ret = WSAStartup(MAKEWORD(2, 2), &sData);
        if ( ret != 0)
        {
            WSACleanup();
            throw wsock_error(ret);
        }

        addrinfo hints;
        addrinfo* aptr;
        memset(&hints, 0, sizeof(hints));
        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_addrlen = 0;
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;
        ret = getaddrinfo("DESKTOP-H9EJU93", "", &hints, &aptr);
        if (ret != 0)
        {
            WSACleanup();
            throw wsock_error(ret);
        }
        addrinfo* pTmp = aptr;
        while (pTmp)
        {
            SOCKADDR_IN* addr = (SOCKADDR_IN*)pTmp->ai_addr;
            AddrInfo aInfo;
            memset(&aInfo, 0, sizeof(aInfo));
            aInfo.addr = pTmp->ai_addr;
            aInfo.addr_len = pTmp->ai_addrlen;
            aInfo.socktype = pTmp->ai_socktype;
            aInfo.protocol = IPPROTO_TCP;
            aInfo.family = pTmp->ai_family;
            inet_ntop(addr->sin_family, &addr->sin_addr, aInfo.ip, 128);

            // 给定IP地址，可以获取主机名
            getnameinfo((SOCKADDR*)addr, sizeof(SOCKADDR) + 16, aInfo.name, 128, NULL, NI_MAXHOST, 0);

            m_AddrInfo.push_back(aInfo);

            pTmp = pTmp->ai_next;
        }

        freeaddrinfo(aptr);
    }

    AddrInfoWrapper::~AddrInfoWrapper()
    {
        WSACleanup();
    }

    AddrInfoWrapper::AddrInfoWrapper()
    {
        Initialized();
    }
}
