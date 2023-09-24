#include "AddrInfoWrapper.h"

namespace RzLib
{
    void AddrInfoWrapper::Initialized()
    {
        WSAData sData; 
        int ret = WSAStartup(MAKEWORD(2, 2), &sData);
        ret = -100;
        if ( ret != 0)
        {
            WSACleanup();
            throw wsock_error(ret);
        }

        addrinfo hints;
        addrinfo* aptr;
        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        ret = getaddrinfo("", "", &hints, &aptr);
        if (ret != 0)
        {
            int err_code = WSAGetLastError();
        }
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
