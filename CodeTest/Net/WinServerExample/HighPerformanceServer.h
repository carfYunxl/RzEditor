#pragma once

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <list>
#include <vector>
#include "AddrInfoWrapper.h"

namespace RzLib
{
    constexpr size_t MAX_OVERLAPPED_RECVS = 200;
    constexpr size_t MAX_COMPLETION_THREAD_COUNT = 32;

    constexpr size_t INITIAL_ACCEPTS = 5;
    constexpr size_t BUFFER_SIZE = 4096;

    constexpr size_t BURST_ACCEPT_COUNT = 100;
    constexpr size_t MAX_OVERLAPPED_ACCEPTS = 500;

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
        struct SOCKET_OBJ*      sock_obj;
    };

    //
    struct LISTEN_OBJ
    {
        SOCKET                      socket;
        int                         AddressFamily;
        std::vector<BUFFER_OBJ*>      PendingAccepts; // Pending AcceptEx buffers
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

        void Run();

    private:
        void ProcessEvent();
        void HandleIO(ULONG_PTR key, BUFFER_OBJ* buf, HANDLE CompPort, DWORD BytesTransfered, DWORD error);

    private:
        std::list<SOCKET_OBJ>   m_sock_objs;
        std::list<BUFFER_OBJ>   m_buff_objs;
        std::vector<LISTEN_OBJ> m_lisn_objs;
        size_t                  mBufferSize{4096};
        std::vector<HANDLE>     m_Events;
        std::vector<AddrInfo>   m_AddrInfo;
        HANDLE                  m_ComPort;

        std::vector<SOCKET_OBJ*>  m_StoreSocketBuffer;
        std::vector<BUFFER_OBJ*>  m_StoreBuffer;
        DWORD                   m_dwProcessNumber;
    };
}
