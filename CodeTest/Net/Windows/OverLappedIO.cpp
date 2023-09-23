#include "OverLappedIO.hpp"
#include "RzCore/Log.hpp"
#include <mswsock.h>
#include <thread>

namespace RzLib
{
    //=====================================================
    // OverLappedIO
    //=====================================================
    bool OverLappedIO::Init()
    {
        WSADATA sData;
        if (WSAStartup((2, 2), &sData) != 0)
        {
            Log(LogLevel::ERR, "WSAStartup() failed with error ", WSAGetLastError());
            return false;
        }

        if ((m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
        {
            Log(LogLevel::ERR, "Failed to get listen socket ", WSAGetLastError());
            return false;
        }

        SOCKADDR_IN sServerAddr;
        sServerAddr.sin_family = AF_INET;
        sServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        sServerAddr.sin_port = htons(PORT);

        if (bind(m_ListenSocket, (PSOCKADDR)&sServerAddr, sizeof(sServerAddr)) == SOCKET_ERROR)
        {
            Log(LogLevel::ERR, "bind failed with error ", WSAGetLastError());
            return false;
        }

        if (listen(m_ListenSocket, 5))
        {
            Log(LogLevel::ERR, "listen failed with error ", WSAGetLastError());
            return false;
        }

        WSAEVENT hEvent = WSACreateEvent();
        if ( hEvent == WSA_INVALID_EVENT )
        {
            Log(LogLevel::ERR, "WSACreateEvent failed with error ", WSAGetLastError());
            return false;
        }
        m_EventVec.push_back(hEvent);

        Log(LogLevel::INFO, "over lapped IO server start to accept client... ", WSAGetLastError());
        return true;
    }

    bool OverLappedIO::Run()
    {
        // Create a thread to service overlapped requests
        std::thread thread(&OverLappedIO::ProcessIO, this);
        thread.detach();

        m_SocketUnitVec.push_back(new SocketUnit);

        while (1)
        {
            // Accept inbound connections
            SOCKET sAcceptSocket = accept(m_ListenSocket, NULL, NULL);
            if (sAcceptSocket == INVALID_SOCKET)
            {
                Log(LogLevel::ERR, "accept failed with error  ", WSAGetLastError());
                return false;
            }
            Log(LogLevel::INFO, "accept socket : ", sAcceptSocket);

            std::lock_guard<std::mutex> locker(m_mutex);

            HANDLE hEvent = WSACreateEvent();
            if (hEvent == WSA_INVALID_EVENT)
            {
                Log(LogLevel::ERR, "WSACreateEvent failed with error  ", WSAGetLastError());
                return false;
            }
            m_EventVec.push_back(hEvent);

            SocketUnit* sUnit = new SocketUnit;
            sUnit->Socket = sAcceptSocket;
            ZeroMemory(&(sUnit->Overlapped), sizeof(OVERLAPPED));
            sUnit->BytesSEND = 0;
            sUnit->BytesRECV = 0;
            sUnit->DataBuf.len = DATA_BUFSIZE;
            sUnit->DataBuf.buf = sUnit->Buffer;
            sUnit->Overlapped.hEvent = hEvent;

            DWORD Flags = 0;
            DWORD RecvBytes = 0;
            if (WSARecv(sUnit->Socket, &(sUnit->DataBuf), 1, &RecvBytes, &Flags, &(sUnit->Overlapped), NULL) == SOCKET_ERROR)
            {
                int ret = WSAGetLastError();
                if (ret != ERROR_IO_PENDING)
                {
                    Log(LogLevel::ERR, "WSARecv failed with error  ", ret);
                    return false;
                }
            }
            m_SocketUnitVec.push_back(sUnit);

            // Signal the first event in the event array to tell the worker thread to
            // service an additional event in the event array
            if (WSASetEvent(m_EventVec[0]) == FALSE)
            {
                Log(LogLevel::ERR, "WSARecv failed with error  ", WSAGetLastError());
                return false;
            }
        }
    }

    void OverLappedIO::ProcessIO()
    {
        DWORD nIndex;
        DWORD BytesTransferred;
        DWORD RecvBytes, SendBytes;
        // Process asynchronous WSASend, WSARecv requests
        while (1)
        {
            if ((nIndex = WSAWaitForMultipleEvents(static_cast<DWORD>(m_EventVec.size()), &m_EventVec[0], FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
            {
                Log(LogLevel::ERR, "WSAWaitForMultipleEvents failed with error  ", WSAGetLastError());
                return;
            }
            Log(LogLevel::INFO, "Get events OK, index = ", nIndex);

            // If the event triggered was zero then a connection attempt was made
            // on our listening socket.
            if ( (nIndex - WSA_WAIT_EVENT_0) == 0 )
            {
                WSAResetEvent(m_EventVec[0]);
                continue;
            }

            SocketUnit* sUnit = m_SocketUnitVec[nIndex - WSA_WAIT_EVENT_0];
            WSAResetEvent(m_EventVec[nIndex - WSA_WAIT_EVENT_0]);

            DWORD Flags = 0;
            if (WSAGetOverlappedResult(sUnit->Socket, &(sUnit->Overlapped), &BytesTransferred, FALSE, &Flags) == FALSE || BytesTransferred == 0)
            {
                Log(LogLevel::INFO, "WSAGetOverlappedResult failed with error ", WSAGetLastError());

                Log(LogLevel::INFO, "Closing socket ", static_cast<int>(sUnit->Socket));

                if (closesocket(sUnit->Socket) == SOCKET_ERROR)
                {
                    Log(LogLevel::ERR, "closesocket failed with error ", WSAGetLastError());
                }

                WSACloseEvent(m_EventVec[nIndex - WSA_WAIT_EVENT_0]);
                // Cleanup SocketArray and EventArray by removing the socket event handle
                // and socket information structure if they are not at the end of the arrays
                std::lock_guard<std::mutex> locker(m_mutex);

                Log(LogLevel::WARN, "delete event and socket : ", sUnit->Overlapped.hEvent, " ", sUnit->Socket);

                std::erase_if(m_EventVec, [=](HANDLE hEvent) {
                    return hEvent == sUnit->Overlapped.hEvent;
                    });

                std::erase_if(m_SocketUnitVec, [=](const SocketUnit* si) {
                    return si->Socket == sUnit->Socket;
                    });
                delete sUnit;
                continue;
            }

            // Check to see if the BytesRECV field equals zero. If this is so, then
            // this means a WSARecv call just completed so update the BytesRECV field
            // with the BytesTransferred value from the completed WSARecv() call.
            if (sUnit->BytesRECV == 0)
            {
                sUnit->BytesRECV = BytesTransferred;
                sUnit->BytesSEND = 0;
            }
            else
            {
                sUnit->BytesSEND += BytesTransferred;
            }

            if (sUnit->BytesRECV > sUnit->BytesSEND)
            {
                // Post another WSASend() request.
                // Since WSASend() is not guaranteed to send all of the bytes requested,
                // continue posting WSASend() calls until all received bytes are sent
                ZeroMemory(&(sUnit->Overlapped), sizeof(WSAOVERLAPPED));
                sUnit->Overlapped.hEvent = m_EventVec[nIndex - WSA_WAIT_EVENT_0];
                sUnit->DataBuf.buf = sUnit->Buffer + sUnit->BytesSEND;
                sUnit->DataBuf.len = sUnit->BytesRECV - sUnit->BytesSEND;

                if (WSASend(sUnit->Socket, &(sUnit->DataBuf), 1, &SendBytes, 0, &(sUnit->Overlapped), NULL) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != ERROR_IO_PENDING)
                    {
                        Log(LogLevel::ERR, "WSASend failed with error ", WSAGetLastError());
                        return;
                    }
                }
            }
            else
            {
                sUnit->BytesRECV = 0;
                // Now that there are no more bytes to send post another WSARecv() request
                Flags = 0;
                ZeroMemory(&(sUnit->Overlapped), sizeof(WSAOVERLAPPED));
                sUnit->Overlapped.hEvent = m_EventVec[ nIndex-WSA_WAIT_EVENT_0 ];
                sUnit->DataBuf.len = DATA_BUFSIZE;
                sUnit->DataBuf.buf = sUnit->Buffer;
                if (WSARecv(sUnit->Socket, &(sUnit->DataBuf), 1, &RecvBytes, &Flags, &(sUnit->Overlapped), NULL) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != ERROR_IO_PENDING)
                    {
                        Log(LogLevel::ERR, "WSARecv failed with error ", WSAGetLastError());
                        return;
                    }
                }
            }
        }
    }
}
