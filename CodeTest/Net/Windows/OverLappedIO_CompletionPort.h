#pragma once

#include "OverLappedIO.hpp"

namespace RzLib{
    class OverLappedIO_CompletionPort
    {
    public:
        struct UnitData
        {
            OVERLAPPED  Overlapped;
            WSABUF      DataBuf;
            CHAR        Buffer[DATA_BUFSIZE];
            DWORD       BytesSEND;
            DWORD       BytesRECV;
        };

        struct UnitHandle
        {
            SOCKET hSocket;
        };
    public:
        OverLappedIO_CompletionPort() = default;
        ~OverLappedIO_CompletionPort() { WSACleanup(); }

        bool Init();
        bool Run();

    private:

        void Runs();
    private:
        SOCKET m_listenSocket;
        HANDLE m_hComPort;
        UnitData* m_pUnitData;
        UnitHandle* m_pUnitHandle;
    };
}
