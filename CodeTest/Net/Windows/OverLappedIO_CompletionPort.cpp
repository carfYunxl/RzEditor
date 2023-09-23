#include "OverLappedIO_CompletionPort.h"

#include "RzCore/Log.hpp"
#include <mswsock.h>
#include <thread>

namespace RzLib
{
    //=====================================================
    // OverLappedIO_CompletionPort
    //=====================================================
    void OverLappedIO_CompletionPort::Runs()
    {
        DWORD nBytesTransferred = 0;
        while (1)
        {
            if (GetQueuedCompletionStatus(m_hComPort, &nBytesTransferred, (PULONG_PTR)&m_pUnitHandle, (LPOVERLAPPED*)&m_pUnitData, INFINITE) == FALSE)
            {
                printf("GetQueuedCompletionStatus() failed with error %d\n", GetLastError());
                return;
            }

            if (nBytesTransferred == 0)
            {
                Log(LogLevel::INFO, "Closing socket ", static_cast<int>(m_pUnitHandle->hSocket));
                if (closesocket(m_pUnitHandle->hSocket) == SOCKET_ERROR)
                {
                    Log(LogLevel::INFO, "closesocket() failed with error ", WSAGetLastError());
                    return;
                }

                delete m_pUnitHandle; m_pUnitHandle = nullptr;
                delete m_pUnitData; m_pUnitData = nullptr;
                continue;
            }

            if (m_pUnitData->BytesRECV == 0)
            {
                m_pUnitData->BytesRECV = nBytesTransferred;
                m_pUnitData->BytesSEND = 0;
            }
            else
            {
                m_pUnitData->BytesSEND += nBytesTransferred;
            }

            if (m_pUnitData->BytesRECV > m_pUnitData->BytesSEND)
            {
                // Post another WSASend() request.
                // Since WSASend() is not guaranteed to send all of the bytes requested,
                // continue posting WSASend() calls until all received bytes are sent.
                ZeroMemory(&(m_pUnitData->Overlapped), sizeof(OVERLAPPED));
                m_pUnitData->DataBuf.buf = m_pUnitData->Buffer + m_pUnitData->BytesSEND;
                m_pUnitData->DataBuf.len = m_pUnitData->BytesRECV - m_pUnitData->BytesSEND;

                DWORD nSendBytes = m_pUnitData->BytesRECV;

                if (WSASend(m_pUnitHandle->hSocket, &(m_pUnitData->DataBuf), 1, &nSendBytes, 0, &(m_pUnitData->Overlapped), NULL) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != ERROR_IO_PENDING)
                    {
                        Log(LogLevel::INFO, "WSASend() failed with error ", WSAGetLastError());
                        return;
                    }
                }
            }
            else
            {
                m_pUnitData->BytesRECV = 0;
                // Now that there are no more bytes to send post another WSARecv() request
                DWORD nFlags = 0, nRecvBytes = 0;
                ZeroMemory(&(m_pUnitData->Overlapped), sizeof(OVERLAPPED));
                m_pUnitData->DataBuf.len = DATA_BUFSIZE;
                m_pUnitData->DataBuf.buf = m_pUnitData->Buffer;

                if (WSARecv(m_pUnitHandle->hSocket, &(m_pUnitData->DataBuf), 1, &nRecvBytes, &nFlags, &(m_pUnitData->Overlapped), NULL) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != ERROR_IO_PENDING)
                    {
                        Log(LogLevel::INFO, "WSARecv() failed with error ", WSAGetLastError());
                        return;
                    }
                }
            }
        }
    }

    bool OverLappedIO_CompletionPort::Init()
    {
        WSADATA wsaData;
        if (int ret = WSAStartup((2, 2), &wsaData) != 0)
        {
            Log(LogLevel::ERR, "WSAStartup failed with error ", ret);
            return false;
        }

        if ((m_hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL)
        {
            Log(LogLevel::ERR, "CreateIoCompletionPort failed with error ", GetLastError());
            return false;
        }

        SYSTEM_INFO SystemInfo;
        GetSystemInfo(&SystemInfo);
        for (int i = 0; i < (int)SystemInfo.dwNumberOfProcessors * 2; i++)
        {
            std::thread thread(&OverLappedIO_CompletionPort::Runs, this);
            thread.detach();
        }

        if ((m_listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
        {
            Log(LogLevel::ERR, "WSASocket failed with error ", WSAGetLastError());
            return false;
        }

        SOCKADDR_IN InternetAddr;
        InternetAddr.sin_family = AF_INET;
        InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        InternetAddr.sin_port = htons(PORT);
        if (bind(m_listenSocket, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
        {
            Log(LogLevel::ERR, "bind failed with error ", WSAGetLastError());
            return false;
        }

        if (listen(m_listenSocket, 5) == SOCKET_ERROR)
        {
            Log(LogLevel::ERR, "listen failed with error ", WSAGetLastError());
            return false;
        }

        return true;
    }

    bool OverLappedIO_CompletionPort::Run()
    {
        while (1)
        {
            SOCKET sAccept = WSAAccept(m_listenSocket, NULL, NULL, NULL, 0);
            if (sAccept == SOCKET_ERROR)
            {
                Log(LogLevel::ERR, "WSAAccept failed with error ", WSAGetLastError());
                return false;
            }

            Log(LogLevel::INFO, "Client connect to server ... ", sAccept);

            m_pUnitHandle = new UnitHandle;
            if (!m_pUnitHandle)
                Log(LogLevel::ERR, "create pUnitHandle failed with error ", GetLastError());

            m_pUnitHandle->hSocket = sAccept;

            if (CreateIoCompletionPort((HANDLE)sAccept, m_hComPort, (ULONG_PTR)m_pUnitHandle, 0) == NULL)
            {
                Log(LogLevel::ERR, "CreateIoCompletionPort failed with error ", WSAGetLastError());
                return false;
            }

            m_pUnitData = new UnitData;
            if (!m_pUnitData)
                Log(LogLevel::ERR, "create pUnitData failed with error ", GetLastError());
            ZeroMemory(&(m_pUnitData->Overlapped), sizeof(OVERLAPPED));
            m_pUnitData->BytesSEND = 0;
            m_pUnitData->BytesRECV = 0;
            m_pUnitData->DataBuf.len = DATA_BUFSIZE;
            m_pUnitData->DataBuf.buf = m_pUnitData->Buffer;

            DWORD nFlags = 0, RecvBytes = 0;
            if (WSARecv(sAccept, &(m_pUnitData->DataBuf), 1, &RecvBytes, &nFlags, &(m_pUnitData->Overlapped), NULL) == SOCKET_ERROR)
            {
                if (WSAGetLastError() != ERROR_IO_PENDING)
                {
                    Log(LogLevel::ERR, "WSARecv failed with error ", GetLastError());
                    return false;
                }
            }
        }


        return true;
    }
}
