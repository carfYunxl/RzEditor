#include "OverLappedIO_AcceptEx.h"

#include "RzCore/Log.hpp"
#include <mswsock.h>
#include <thread>

namespace RzLib
{
    //=====================================================
    // OverLappedIO_AcceptEx
    //=====================================================
    bool OverLappedIO_AcceptEx::Init()
    {
        WSADATA wsaData;
        if ((WSAStartup((2, 2), &wsaData)) != 0)
        {
            Log(LogLevel::ERR, "WSAStartup() failed with error ", WSAGetLastError());
            return false;
        }

        if ((m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
        {
            Log(LogLevel::ERR, "failed to create listen socket , error %d\n", WSAGetLastError());
            return false;
        }

        SOCKADDR_IN InternetAddr;
        InternetAddr.sin_family = AF_INET;
        InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        InternetAddr.sin_port = htons(PORT);

        if (bind(m_ListenSocket, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
        {
            Log(LogLevel::ERR, "bind failed with error %d\n", WSAGetLastError());
            return false;
        }

        if (listen(m_ListenSocket, 5) == SOCKET_ERROR)
        {
            Log(LogLevel::ERR, "listen failed with error %d\n", WSAGetLastError());
            return false;
        }

        if ((m_AcceptSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
        {
            Log(LogLevel::ERR, "failed to create accept socket , error %d\n", WSAGetLastError());
            return false;
        }
        return true;
    }

    bool OverLappedIO_AcceptEx::Run()
    {
        m_SocketUnitVec.push_back(new SocketUnit);

        CHAR AcceptBuffer[2 * (sizeof(SOCKADDR_IN) + 16)];
        DWORD Bytes = 0;
        WSAOVERLAPPED ListenOverlapped;
        ZeroMemory(&ListenOverlapped, sizeof(OVERLAPPED));
        WSAEVENT hEvent = WSACreateEvent();
        if (hEvent == WSA_INVALID_EVENT)
        {
            Log(LogLevel::ERR, "WSACreateEvent failed with error ", WSAGetLastError());
            return false;
        }
        m_EventVec.push_back(hEvent);
        ListenOverlapped.hEvent = hEvent;

        if (AcceptEx(m_ListenSocket, m_AcceptSocket, (PVOID)AcceptBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &Bytes, &ListenOverlapped) == FALSE)
        {
            if (WSAGetLastError() != ERROR_IO_PENDING)
            {
                Log(LogLevel::ERR, "AcceptEx failed with error ", WSAGetLastError());
                return false;
            }
        }

        while (1)
        {
            DWORD nIndex = 0;
            if ((nIndex = WSAWaitForMultipleEvents(static_cast<DWORD>(m_EventVec.size()), &m_EventVec[0], FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
            {
                Log(LogLevel::ERR, "WSAWaitForMultipleEvents failed with error ", WSAGetLastError());
                return false;
            }

            DWORD Flags = 0, BytesTransferred = 0;
            if ((nIndex - WSA_WAIT_EVENT_0) == 0)
            {
                // Check the returns from the overlapped I/O operation on the listening socket
                if (WSAGetOverlappedResult(m_ListenSocket, &(ListenOverlapped), &BytesTransferred, FALSE, &Flags) == FALSE)
                {
                    Log(LogLevel::WARN, "WSAGetOverlappedResult failed with error ", WSAGetLastError());
                    return false;
                }

                Log(LogLevel::INFO, "client connect to server...", m_AcceptSocket);
                if (m_EventVec.size() > WSA_MAXIMUM_WAIT_EVENTS)
                {
                    Log(LogLevel::WARN, "Too many connections - closing socket.");
                    closesocket(m_AcceptSocket);
                    continue;
                }

                SocketUnit* sInfo = new SocketUnit;
                if (!sInfo)
                {
                    Log(LogLevel::ERR, "new SockInfo failed with error ", GetLastError());
                    return false;
                }
                sInfo->Socket = m_AcceptSocket;
                ZeroMemory(&(sInfo->Overlapped), sizeof(OVERLAPPED));
                sInfo->BytesSEND = 0;
                sInfo->BytesRECV = 0;
                sInfo->DataBuf.len = DATA_BUFSIZE;
                sInfo->DataBuf.buf = sInfo->Buffer;

                WSAEVENT hEvent = WSACreateEvent();
                if (hEvent == WSA_INVALID_EVENT)
                {
                    Log(LogLevel::ERR, "WSACreateEvent failed with error ", WSAGetLastError());
                    return false;
                }
                sInfo->Overlapped.hEvent = hEvent;
                m_EventVec.push_back(hEvent);
                m_SocketUnitVec.push_back(sInfo);

                // Post a WSARecv request to to begin receiving data on the socket
                DWORD RecvBytes = 0;
                if (WSARecv(sInfo->Socket, &(sInfo->DataBuf), 1, &RecvBytes, &Flags, &(sInfo->Overlapped), NULL) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != ERROR_IO_PENDING)
                    {
                        Log(LogLevel::ERR, "WSARecv failed with error ", WSAGetLastError());
                        return FALSE;
                    }
                }

                // Make a new socket for accepting future connections and post another AcceptEx call
                if ((m_AcceptSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
                {
                    Log(LogLevel::ERR, "create accept socket failed with error ", WSAGetLastError());
                    return false;
                }
                WSAResetEvent(m_EventVec[0]);
                ZeroMemory(&ListenOverlapped, sizeof(OVERLAPPED));
                ListenOverlapped.hEvent = m_EventVec[0];

                if (AcceptEx(m_ListenSocket, m_AcceptSocket, (PVOID)AcceptBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &Bytes, &ListenOverlapped) == FALSE)
                {
                    if (WSAGetLastError() != ERROR_IO_PENDING)
                    {
                        Log(LogLevel::ERR, "AcceptEx failed with error ", WSAGetLastError());
                        return false;
                    }
                }
                continue;
            }

            //
            SocketUnit* SI = m_SocketUnitVec[nIndex - WSA_WAIT_EVENT_0];
            WSAResetEvent(m_EventVec[nIndex - WSA_WAIT_EVENT_0]);
            if (WSAGetOverlappedResult(SI->Socket, &(SI->Overlapped), &BytesTransferred, FALSE, &Flags) == FALSE)
            {
                Log(LogLevel::ERR, "WSAGetOverlappedResult() failed with error ", WSAGetLastError());

                // First check to see if the peer has closed the connection and if so
                // then close the socket and cleanup the SOCKET_INFORMATION structure
                // associated with the socket
                if (BytesTransferred == 0)
                {
                    Log(LogLevel::ERR, "WSAGetOverlappedResult() failed with error ", WSAGetLastError());
                    if (closesocket(SI->Socket) == SOCKET_ERROR)
                    {
                        Log(LogLevel::ERR, "closesocket() failed with error ", WSAGetLastError());
                    }
                    Log(LogLevel::INFO, "Closing socket ", static_cast<int>(SI->Socket));

                    WSACloseEvent(m_EventVec[nIndex - WSA_WAIT_EVENT_0]);
                    // Cleanup SocketArray and EventArray by removing the socket event handle
                    // and socket information structure if they are not at the end of the arrays
                    std::erase_if(m_EventVec, [=](HANDLE hEvent) {
                        return hEvent == SI->Overlapped.hEvent;
                        });

                    std::erase_if(m_SocketUnitVec, [=](const SocketUnit* si) {
                        return si->Socket == SI->Socket;
                        });
                    delete SI;
                    continue;
                }
            }

            // Check to see if the BytesRECV field equals zero. If this is so, then
            // this means a WSARecv call just completed so update the BytesRECV field
            // with the BytesTransferred value from the completed WSARecv() call
            if (SI->BytesRECV == 0)
            {
                SI->BytesRECV = BytesTransferred;
                SI->BytesSEND = 0;
            }
            else
            {
                SI->BytesSEND += BytesTransferred;
            }

            DWORD SendBytes = 0, RecvBytes = 0;
            if (SI->BytesRECV > SI->BytesSEND)
            {
                // Post another WSASend() request
                // Since WSASend() is not guaranteed to send all of the bytes requested,
                // continue posting WSASend() calls until all received bytes are sent
                ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
                SI->Overlapped.hEvent = m_EventVec[nIndex - WSA_WAIT_EVENT_0];
                SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
                SI->DataBuf.len = SI->BytesRECV - SI->BytesSEND;

                if (WSASend(SI->Socket, &(SI->DataBuf), 1, &SendBytes, 0, &(SI->Overlapped), NULL) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != ERROR_IO_PENDING)
                    {
                        Log(LogLevel::ERR, "WSASend() failed with error ", WSAGetLastError());
                        return false;
                    }
                }
            }
            else
            {
                SI->BytesRECV = 0;
                // Now that there are no more bytes to send post another WSARecv() request
                Flags = 0;
                ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
                SI->Overlapped.hEvent = m_EventVec[nIndex - WSA_WAIT_EVENT_0];
                SI->DataBuf.len = DATA_BUFSIZE;
                SI->DataBuf.buf = SI->Buffer;

                if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags, &(SI->Overlapped), NULL) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != ERROR_IO_PENDING)
                    {
                        Log(LogLevel::ERR, "WSARecv() failed with error ", WSAGetLastError());
                        return false;
                    }
                }
            }
        }

        return true;
    }
}
