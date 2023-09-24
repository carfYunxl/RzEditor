#pragma once

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <list>
#include <vector>

namespace RzLib
{
    constexpr size_t MAX_OVERLAPPED_RECVS = 200;
    constexpr size_t MAX_COMPLETION_THREAD_COUNT = 32;

    enum class OPERATION
    {
        OP_ACCEPT = 0,  // AcceptEx
        OP_READ,        // WSARecv/WSARecvFrom
        OP_WRITE        // WSASend/WSASendTo
    };

    // This is our per socket buffer. It contains information about the socket handle
    //which is returned from each GetQueuedCompletionStatus call.
    struct SOCKET_OBJ
    {
        SOCKET              socket;             // Socket handle
        int                 af;                 // Address family of socket (AF_INET, AF_INET6)
        bool                bClosing;           // Is the socket closing?
        volatile LONG       OutstandingRecv;    // Number of outstanding overlapped ops on
        volatile LONG       OutstandingSend;
        volatile LONG       PendingSend;
    } ;

    struct BUFFER_OBJ
    {
        WSAOVERLAPPED           ol;
        SOCKET                  sclient;       // Used for AcceptEx client socket
        HANDLE                  PostAccept;
        char*                   buf;           // Buffer for recv/send/AcceptEx
        int                     buflen;        // Length of the buffer
        int                     operation;     // Type of operation issued
        SOCKADDR_STORAGE        addr;
        int                     addrlen;
        struct SOCKET_OBJ*      sock;
    };

    //
    struct LISTEN_OBJ
    {
        SOCKET                      socket;
        int                         AddressFamily;
        BUFFER_OBJ*                 PendingAccepts; // Pending AcceptEx buffers
        volatile long               PendingAcceptCount;
        int                         HiWaterMark, LoWaterMark;
        HANDLE                      AcceptEvent;
        HANDLE                      RepostAccept;
        volatile long               RepostCount;

        LPFN_ACCEPTEX               lpfnAcceptEx;
        LPFN_GETACCEPTEXSOCKADDRS   lpfnGetAcceptExSockaddrs;
    };

    class HighPerformanceServer
    {
    public:
        bool Init();

    private:
        void ProcessEvent(HANDLE nComPort);

    private:
        std::list<SOCKET_OBJ> m_sock_objs;
        std::list<BUFFER_OBJ> m_buff_objs;
        std::list<LISTEN_OBJ> m_lisn_objs;

        size_t mBufferSize{4096};

        std::vector<HANDLE> m_Events;
    };
}
