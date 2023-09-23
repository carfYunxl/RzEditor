#pragma once

#include "OverLappedIO.hpp"

namespace RzLib
{
    class OverLappedIO_AcceptEx
    {
    public:
        OverLappedIO_AcceptEx() = default;
        ~OverLappedIO_AcceptEx() { WSACleanup(); }

        bool Init();
        bool Run();

    private:
        SOCKET                      m_ListenSocket;
        SOCKET                      m_AcceptSocket;
        std::vector<WSAEVENT>       m_EventVec;
        std::mutex                  m_mutex;
        std::vector<SocketUnit*>    m_SocketUnitVec;
    };
}
