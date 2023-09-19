#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#include <stdio.h>

#define PORT 8080
#define DATA_BUFSIZE 8192

struct SockInfo
{
    CHAR Buffer[DATA_BUFSIZE];
    WSABUF DataBuf;
    SOCKET Socket;
    WSAOVERLAPPED Overlapped;
    DWORD BytesSEND;
    DWORD BytesRECV;
};

int main5(int argc, char** argv)
{
    DWORD EventTotal = 0;
    WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
    SockInfo* SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
    CHAR AcceptBuffer[2 * (sizeof(SOCKADDR_IN) + 16)];
    WSAOVERLAPPED ListenOverlapped;
    DWORD Bytes;
    DWORD Index;
    DWORD Flags;
    DWORD BytesTransferred;
    SOCKET ListenSocket, AcceptSocket;
    SOCKADDR_IN InternetAddr;
    DWORD RecvBytes, SendBytes;
    DWORD i;


    WSADATA wsaData;
    if ((WSAStartup((2, 2), &wsaData)) != 0)
    {
        printf("WSAStartup() failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("WSAStartup() is OK!\n");

    if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        printf("Failed to get a socket %d\n", WSAGetLastError());
        return 1;
    }
    printf("WSASocket() is OK!\n");

    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(PORT);

    if (bind(ListenSocket, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
    {
        printf("bind() failed with error %d\n", WSAGetLastError());
        return 1;
    }
    printf("bind() is OK!\n");

    if (listen(ListenSocket, 5) == SOCKET_ERROR)
    {
        printf("listen() failed with error %d\n", WSAGetLastError());
        return 1;
    }
    printf("listen() is OK! I am listening...\n");

    // Setup the listening socket for connections
    if ((AcceptSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        printf("Failed to get a socket %d\n", WSAGetLastError());
        return 1;
    }
    printf("WSASocket() is OK!\n");

    ZeroMemory(&ListenOverlapped, sizeof(OVERLAPPED));
    if ((EventArray[0] = ListenOverlapped.hEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
        return 1;
    }
    printf("WSACreateEvent() is OK!\n");

    EventTotal = 1;

    if (AcceptEx(ListenSocket, AcceptSocket, (PVOID)AcceptBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &Bytes, &ListenOverlapped) == FALSE)
    {
        if (WSAGetLastError() != ERROR_IO_PENDING)
        {
            printf("AcceptEx() failed with error %d\n", WSAGetLastError());
            return 1;
        }
        printf("AcceptEx() is OK!\n");
    }

    // Process asynchronous AcceptEx, WSASend, WSARecv requests
    while (1)
    {
        if ((Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
        {
            printf("WSAWaitForMultipleEvents() failed %d\n", WSAGetLastError());
            return 1;
        }
        printf("WSAWaitForMultipleEvents() is OK!\n");

        // If the event triggered was zero then a connection attempt was made
        // on our listening socket
        if ((Index - WSA_WAIT_EVENT_0) == 0)
        {
            // Check the returns from the overlapped I/O operation on the listening socket
            if (WSAGetOverlappedResult(ListenSocket, &(ListenOverlapped), &BytesTransferred, FALSE, &Flags) == FALSE)
            {
                printf("WSAGetOverlappedResult() failed with error %d\n", WSAGetLastError());
                return 1;
            }
            printf("WSAGetOverlappedResult() is OK!\n");

            printf("Socket %d got connected...\n", AcceptSocket);
            if (EventTotal > WSA_MAXIMUM_WAIT_EVENTS)
            {
                printf("Too many connections - closing socket.\n");
                closesocket(AcceptSocket);
                continue;
            }
            else
            {
                SockInfo* sInfo = new SockInfo;
                SocketArray[EventTotal] = sInfo;
                // Create a socket information structure to associate with the accepted socket
                if (!sInfo)
                {
                    printf("new SockInfo failed with error %d\n", GetLastError());
                    return 1;
                }
                printf("GlobalAlloc() for LPSOCKET_INFORMATION is OK!\n");

                // Fill in the details of our accepted socket
                SocketArray[EventTotal]->Socket = AcceptSocket;
                ZeroMemory(&(SocketArray[EventTotal]->Overlapped), sizeof(OVERLAPPED));
                SocketArray[EventTotal]->BytesSEND = 0;
                SocketArray[EventTotal]->BytesRECV = 0;
                SocketArray[EventTotal]->DataBuf.len = DATA_BUFSIZE;
                SocketArray[EventTotal]->DataBuf.buf = SocketArray[EventTotal]->Buffer;

                if ((SocketArray[EventTotal]->Overlapped.hEvent = EventArray[EventTotal] = WSACreateEvent()) == WSA_INVALID_EVENT)
                {
                    printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
                    return 1;
                }
                printf("WSACreateEvent() is OK!\n");

                // Post a WSARecv request to to begin receiving data on the socket
                if (WSARecv(SocketArray[EventTotal]->Socket, &(SocketArray[EventTotal]->DataBuf), 1, &RecvBytes, &Flags, &(SocketArray[EventTotal]->Overlapped), NULL) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != ERROR_IO_PENDING)
                    {
                        printf("WSARecv() failed with error %d\n", WSAGetLastError());
                        return 1;
                    }
                }
                printf("WSARecv() is OK!\n");
                EventTotal++;
            }

            // Make a new socket for accepting future connections and post another AcceptEx call
            if ((AcceptSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
            {
                printf("Failed to get a socket %d\n", WSAGetLastError());
                return 1;
            }
            printf("WSASocket() is OK!\n");

            WSAResetEvent(EventArray[0]);
            ZeroMemory(&ListenOverlapped, sizeof(OVERLAPPED));
            ListenOverlapped.hEvent = EventArray[0];

            if (AcceptEx(ListenSocket, AcceptSocket, (PVOID)AcceptBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &Bytes, &ListenOverlapped) == FALSE)
            {
                if (WSAGetLastError() != ERROR_IO_PENDING)
                {
                    printf("AcceptEx() failed with error %d\n", WSAGetLastError());
                    return 1;
                }
            }
            printf("AcceptEx() is OK!\n");
            continue;
        }

        SockInfo* SI = SocketArray[Index - WSA_WAIT_EVENT_0];
        WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
        if (WSAGetOverlappedResult(SI->Socket, &(SI->Overlapped), &BytesTransferred, FALSE, &Flags) == FALSE)
        {
            printf("WSAGetOverlappedResult() failed with error %d\n", WSAGetLastError());
            return 1;
        }
        printf("WSAGetOverlappedResult() is OK!\n");

        // First check to see if the peer has closed the connection and if so
        // then close the socket and cleanup the SOCKET_INFORMATION structure
        // associated with the socket
        if (BytesTransferred == 0)
        {
            printf("Closing socket %d\n", static_cast<int>(SI->Socket));
            if (closesocket(SI->Socket) == SOCKET_ERROR)
            {
                printf("closesocket() failed with error %d\n", WSAGetLastError());
            }
            printf("closesocket() is OK!\n");

            delete SI;
            WSACloseEvent(EventArray[Index - WSA_WAIT_EVENT_0]);
            // Cleanup SocketArray and EventArray by removing the socket event handle
            // and socket information structure if they are not at the end of the arrays
            if ((Index - WSA_WAIT_EVENT_0) + 1 != EventTotal)
                for (i = Index - WSA_WAIT_EVENT_0; i < EventTotal; i++)
                {
                    EventArray[i] = EventArray[i + 1];
                    SocketArray[i] = SocketArray[i + 1];
                }

            EventTotal--;
            continue;
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

        if (SI->BytesRECV > SI->BytesSEND)
        {
            // Post another WSASend() request
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
                    return 1;
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
                    return 1;
                }
            }
            else
                printf("WSARecv() is OK!\n");
        }
    }
}