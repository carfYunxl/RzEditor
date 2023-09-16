#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <mutex>
#include <array>
#include <thread>

#define PORT 8080
#define DATA_BUFSIZE 8192

struct SOCK_INFO
{
    CHAR Buffer[DATA_BUFSIZE];
    WSABUF DataBuf;
    SOCKET Socket;
    WSAOVERLAPPED Overlapped;
    DWORD BytesSEND;
    DWORD BytesRECV;
};

using pSOCK_INFO = SOCK_INFO*;

DWORD WINAPI ProcessIO(LPVOID lpParameter);

DWORD dwTotalEvent = 0;

std::array<WSAEVENT, WSA_MAXIMUM_WAIT_EVENTS> EventArray;

pSOCK_INFO SocketArray[WSA_MAXIMUM_WAIT_EVENTS];

std::mutex g_mutex;

void Process()
{
    DWORD Index;
    pSOCK_INFO SI;
    DWORD BytesTransferred;
    DWORD i;
    DWORD RecvBytes, SendBytes;
    // Process asynchronous WSASend, WSARecv requests
    while (TRUE)
    {
        if ((Index = WSAWaitForMultipleEvents(dwTotalEvent, &EventArray[0], FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
        {
            printf("WSAWaitForMultipleEvents() failed %d\n", WSAGetLastError());
            return;
        }
        printf("WSAWaitForMultipleEvents() is OK!\n");

        // If the event triggered was zero then a connection attempt was made
        // on our listening socket.
        if ((Index - WSA_WAIT_EVENT_0) == 0)
        {
            WSAResetEvent(EventArray[0]);
            continue;
        }

        SI = SocketArray[Index - WSA_WAIT_EVENT_0];
        WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
        DWORD Flags = 0;
        if (WSAGetOverlappedResult(SI->Socket, &(SI->Overlapped), &BytesTransferred, FALSE, &Flags) == FALSE || BytesTransferred == 0)
        {
            printf("Closing socket %d\n", static_cast<int>(SI->Socket));

            if (closesocket(SI->Socket) == SOCKET_ERROR)
            {
                printf("closesocket() failed with error %d\n", WSAGetLastError());
            }
            else
                printf("closesocket() is OK!\n");

            delete SI;
            WSACloseEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
            // Cleanup SocketArray and EventArray by removing the socket event handle
            // and socket information structure if they are not at the end of the arrays
            std::lock_guard<std::mutex> locker(g_mutex);
            if ((Index - WSA_WAIT_EVENT_0) + 1 != dwTotalEvent)
                for (i = Index - WSA_WAIT_EVENT_0; i < dwTotalEvent; i++)
                {
                    EventArray[i] = EventArray[i + 1];
                    SocketArray[i] = SocketArray[i + 1];
                }
            dwTotalEvent--;
            continue;
        }

        // Check to see if the BytesRECV field equals zero. If this is so, then
        // this means a WSARecv call just completed so update the BytesRECV field
        // with the BytesTransferred value from the completed WSARecv() call.
        if (SI->BytesRECV == 0)
        {
            SI->BytesRECV = BytesTransferred;
            SI->BytesSEND = 0;
        }
        else
        {
            SI->BytesSEND += BytesTransferred;
        }

        if (SI->BytesRECV > SI->BytesSEND)
        {
            // Post another WSASend() request.
            // Since WSASend() is not guaranteed to send all of the bytes requested,
            // continue posting WSASend() calls until all received bytes are sent
            ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
            SI->Overlapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];
            SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
            SI->DataBuf.len = SI->BytesRECV - SI->BytesSEND;

            if (WSASend(SI->Socket, &(SI->DataBuf), 1, &SendBytes, 0, &(SI->Overlapped), NULL) == SOCKET_ERROR)
            {
                if (WSAGetLastError() != ERROR_IO_PENDING)
                {
                    printf("WSASend() failed with error %d\n", WSAGetLastError());
                    return;
                }
            }
            else
                printf("WSASend() is OK!\n");
        }
        else
        {
            SI->BytesRECV = 0;
            // Now that there are no more bytes to send post another WSARecv() request
            Flags = 0;
            ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
            SI->Overlapped.hEvent = EventArray[Index - WSA_WAIT_EVENT_0];
            SI->DataBuf.len = DATA_BUFSIZE;
            SI->DataBuf.buf = SI->Buffer;
            if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags, &(SI->Overlapped), NULL) == SOCKET_ERROR)
            {
                if (WSAGetLastError() != ERROR_IO_PENDING)
                {
                    printf("WSARecv() failed with error %d\n", WSAGetLastError());
                    return;
                }
            }
            else
                printf("WSARecv() is OK!\n");
        }
    }
}

int main(int argc, char** argv)
{
    WSADATA sData;
    SOCKET sListenSocket, sAcceptSocket;
    SOCKADDR_IN sServerAddr;
    DWORD RecvBytes;

    if (WSAStartup((2, 2), &sData) != 0)
    {
        printf("WSAStartup() failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("WSAStartup() looks nice!\n");

    if ((sListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        printf("Failed to get a socket %d\n", WSAGetLastError());
        return 1;
    }
    printf("WSASocket() is OK lol!\n");

    sServerAddr.sin_family = AF_INET;
    sServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sServerAddr.sin_port = htons(PORT);

    if (bind(sListenSocket, (PSOCKADDR)&sServerAddr, sizeof(sServerAddr)) == SOCKET_ERROR)
    {
        printf("bind() failed with error %d\n", WSAGetLastError());
        return 1;
    }
    printf("YOu see, bind() is working!\n");

    if (listen(sListenSocket, 5))
    {
        printf("listen() failed with error %d\n", WSAGetLastError());
        return 1;
    }
    printf("listen() is OK maa...\n");

    if ((EventArray[0] = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
        return 1;
    }
    printf("WSACreateEvent() is OK!\n");

    // Create a thread to service overlapped requests
    std::thread thread(Process);
    thread.detach();

    dwTotalEvent = 1;

    while (TRUE)
    {
        // Accept inbound connections
        if ((sAcceptSocket = accept(sListenSocket, NULL, NULL)) == INVALID_SOCKET)
        {
            printf("accept() failed with error %d\n", WSAGetLastError());
            return 1;
        }
        printf("accept() is OK!\n");

        std::lock_guard<std::mutex> locker(g_mutex);
        pSOCK_INFO sInfo = new SOCK_INFO;
        if (!sInfo)
        {
            printf("alloc new SOCK_INFO failed with error %d\n", GetLastError());
            return 1;
        }
        printf("alloc new SOCK_INFO is pretty fine!\n");

        // Fill in the details of our accepted socket
        sInfo->Socket = sAcceptSocket;
        ZeroMemory(&(sInfo->Overlapped), sizeof(OVERLAPPED));
        sInfo->BytesSEND = 0;
        sInfo->BytesRECV = 0;
        sInfo->DataBuf.len = DATA_BUFSIZE;
        sInfo->DataBuf.buf = sInfo->Buffer;

        HANDLE hEvent = WSACreateEvent();
        if ( hEvent == WSA_INVALID_EVENT )
        {
            printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
            return 1;
        }
        sInfo->Overlapped.hEvent = hEvent;
        EventArray[dwTotalEvent] = hEvent;
        printf("WSACreateEvent() is OK!\n");

        DWORD Flags = 0;
        if (WSARecv(sInfo->Socket, &(sInfo->DataBuf), 1, &RecvBytes, &Flags, &(sInfo->Overlapped), NULL) == SOCKET_ERROR)
        {
            int ret = WSAGetLastError();
            if (ret != ERROR_IO_PENDING)
            {
                printf("WSARecv() failed with error %d\n", ret);
                return 1;
            }
        }
        printf("WSARecv() should be working!\n");

        SocketArray[dwTotalEvent] = sInfo;
        dwTotalEvent++;

        // Signal the first event in the event array to tell the worker thread to
        // service an additional event in the event array
        if (WSASetEvent(EventArray[0]) == FALSE)
        {
            printf("WSASetEvent() failed with error %d\n", WSAGetLastError());
            return 1;
        }
        printf("Don't worry, WSASetEvent() is OK!\n");
    }
}
