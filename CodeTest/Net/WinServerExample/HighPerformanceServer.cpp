#include "HighPerformanceServer.h"
#include "RzCore/Log.hpp"
#include <thread>

namespace RzLib
{
    void HighPerformanceServer::Init()
    {
        WSADATA sData;
        int ret = WSAStartup(MAKEWORD(2, 2), &sData);
        if ( ret != 0)
        {
            Log(LogLevel::ERR, "fail to load Winsock!, error = ", ret);
            throw wsock_error(ret);
        }

        m_ComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)NULL, 0);
        if (m_ComPort == NULL)
        {
            throw wsock_error(GetLastError());
        }

        SYSTEM_INFO sInfo;
        GetSystemInfo(&sInfo);
        if (sInfo.dwNumberOfProcessors > MAX_COMPLETION_THREAD_COUNT)
        {
            sInfo.dwNumberOfProcessors = MAX_COMPLETION_THREAD_COUNT;
        }

        m_dwProcessNumber = sInfo.dwNumberOfProcessors;

        // Round the buffer size to the next increment of the page size
        if ((mBufferSize % sInfo.dwPageSize) != 0)
        {
            mBufferSize = ((mBufferSize / sInfo.dwPageSize) + 1) * sInfo.dwPageSize;
        }
        Log(LogLevel::INFO, "Buffer size = ", mBufferSize, "page size = ", sInfo.dwPageSize);

        for (int i = 0; i < (int)sInfo.dwNumberOfProcessors; i++)
        {
            std::thread th(&HighPerformanceServer::ProcessEvent, this);
            HANDLE hHandle = static_cast<HANDLE>(th.native_handle());
            m_Events.push_back(hHandle);
            th.detach();
        }

        AddrInfoWrapper wrapper;

        m_AddrInfo = std::move(wrapper.GetAllAdrInfo());

        if (m_AddrInfo.empty())
        {
            throw wsock_error(GetLastError());
        }
    }

    void HighPerformanceServer::Run()
    {
        for (size_t i = 0; i < m_AddrInfo.size(); ++i)
        {
            SOCKET soc = socket(m_AddrInfo[i].family, m_AddrInfo[i].socktype, m_AddrInfo[i].protocol);
            if (soc == INVALID_SOCKET)
            {
                throw wsock_error(WSAGetLastError());
            }

            HANDLE hAcceptEvent = CreateEvent(NULL, TRUE, FALSE, NULL); //此API可以自动重置事件，而WSACreateEvent需要手动重置事件
            if (hAcceptEvent == NULL)
            {
                throw wsock_error(GetLastError());
            }

            HANDLE hRepostEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (hRepostEvent == NULL)
            {
                throw wsock_error(GetLastError());
            }

            LISTEN_OBJ listen_obj;
            listen_obj.LoWaterMark = 5;
            listen_obj.AddressFamily = m_AddrInfo[i].family;
            listen_obj.AcceptEvent = hAcceptEvent;
            listen_obj.RepostAccept = hRepostEvent;
            listen_obj.socket = soc;

            m_Events.push_back(hAcceptEvent);
            m_Events.push_back(hRepostEvent);

            HANDLE hCP = CreateIoCompletionPort(
                (HANDLE)listen_obj.socket,
                m_ComPort,
                (ULONG_PTR)&listen_obj,
                0
                );
            if (hCP == NULL)
            {
                throw wsock_error(GetLastError());
            }

            int ret = bind(listen_obj.socket, m_AddrInfo[i].addr, static_cast<int>(m_AddrInfo[i].addr_len));
            if (ret == SOCKET_ERROR)
            {
                throw wsock_error(WSAGetLastError());
            }

            GUID guidAcceptEx = WSAID_ACCEPTEX;
            GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
            DWORD bytes;
            ret = WSAIoctl(
                listen_obj.socket,
                SIO_GET_EXTENSION_FUNCTION_POINTER,
                &guidAcceptEx,
                sizeof(guidAcceptEx),
                &listen_obj.lpfnAcceptEx,
                sizeof(listen_obj.lpfnAcceptEx),
                &bytes,
                NULL,
                NULL
            );
            if (ret == SOCKET_ERROR)
            {
                throw wsock_error(WSAGetLastError());
            }

            ret = WSAIoctl(
                listen_obj.socket,
                SIO_GET_EXTENSION_FUNCTION_POINTER,
                &guidGetAcceptExSockaddrs,
                sizeof(guidGetAcceptExSockaddrs),
                &listen_obj.lpfnGetAcceptExSockaddrs,
                sizeof(listen_obj.lpfnGetAcceptExSockaddrs),
                &bytes,
                NULL,
                NULL
            );
            if (ret == SOCKET_ERROR)
            {
                throw wsock_error(WSAGetLastError());
            }

            ret = listen(listen_obj.socket, 200);
            if (ret == SOCKET_ERROR)
            {
                throw wsock_error(WSAGetLastError());
            }

            ret = WSAEventSelect(listen_obj.socket, listen_obj.AcceptEvent, FD_ACCEPT);
            if (ret == SOCKET_ERROR)
            {
                throw wsock_error(WSAGetLastError());
            }

            BUFFER_OBJ* buffer_obj = nullptr;
            for (size_t i = 0;i < INITIAL_ACCEPTS; ++i)
            {
                if (m_StoreBuffer.empty())
                    buffer_obj = new BUFFER_OBJ;
                else
                    buffer_obj = m_StoreBuffer.back();

                buffer_obj->PostAccept = listen_obj.AcceptEvent;

                listen_obj.PendingAccepts.push_back(buffer_obj);

                buffer_obj->sclient = socket(listen_obj.AddressFamily, SOCK_STREAM, IPPROTO_TCP);
                buffer_obj->buf = (char*)((char*)(buffer_obj) + sizeof(BUFFER_OBJ));
                buffer_obj->buflen = BUFFER_SIZE;
                buffer_obj->operation = static_cast<int>(OPERATION::OP_ACCEPT);
                if (buffer_obj->sclient == INVALID_SOCKET)
                {
                    throw wsock_error(WSAGetLastError());
                }

                BOOL rb = listen_obj.lpfnAcceptEx(
                    listen_obj.socket,
                    buffer_obj->sclient,
                    buffer_obj->buf,
                    buffer_obj->buflen - ((sizeof(SOCKADDR_STORAGE) + 16) * 2),
                    sizeof(SOCKADDR_STORAGE) + 16,
                    sizeof(SOCKADDR_STORAGE) + 16,
                    &bytes,
                    &buffer_obj->ol
                );

                if (rb == FALSE)
                {
                    if (WSAGetLastError() != WSA_IO_PENDING )
                    {
                        throw wsock_error(WSAGetLastError());
                    }
                }

                listen_obj.PendingAcceptCount++;
                //endpointcount++;
                m_lisn_objs.push_back(listen_obj);
            }
        }

        int interval = 0;
        while (1)
        {
            int ret = WSAWaitForMultipleEvents(m_Events.size(), &m_Events[0], FALSE, 5000, FALSE);
            if (ret == WAIT_FAILED)
            {
                throw wsock_error(WSAGetLastError());
            }

            if (ret == WAIT_TIMEOUT)
            {
                interval++;

                if (interval == 36)
                {
                    // For TCP, cycle through all the outstanding AcceptEx operations
                    //   to see if any of the client sockets have been connected but
                    //   haven't received any data. If so, close them as they could be
                    //   a denial of service attack.
                    for (size_t i = 0; i < m_lisn_objs.size();++i)
                    {
                        std::vector<BUFFER_OBJ*> vecbufferObj = m_lisn_objs.at(i).PendingAccepts;

                        for (size_t j = 0; j < vecbufferObj.size(); ++j)
                        {
                            int optval = 0;
                            int optlen = sizeof(optval);

                            ret = getsockopt(vecbufferObj.at(i)->sclient, SOL_SOCKET, SO_CONNECT_TIME, (char*)&optval, &optlen);
                            if (ret == SOCKET_ERROR)
                            {
                                throw wsock_error(WSAGetLastError());
                            }
                            else
                            {
                                // If the socket has been connected for more than 5 minutes,
                                //    close it. If closed, the AcceptEx call will fail in the completion thread.
                                if ((optval != 0xFFFFFFFF) && (optval > 300))
                                {
                                    printf("closing stale handle\n");
                                    closesocket(vecbufferObj.at(i)->sclient);
                                    vecbufferObj.at(i)->sclient = INVALID_SOCKET;
                                }
                            }
                        }
                    }
                    interval = 0;
                }
            }
            else
            {
                int index = ret - WAIT_OBJECT_0;

                for (size_t i = 0;i < m_Events.size(); ++i)
                {
                    ret = WaitForSingleObject(m_Events.at(index), 0);
                    if (ret == WAIT_FAILED || ret == WAIT_TIMEOUT)
                    {
                        continue;
                    }

                    if (index < m_dwProcessNumber)
                    {
                        // One of the completion threads exited
                        //   This is bad so just bail - a real server would want
                        //   to gracefully exit but this is just a sample ...
                        ExitProcess(-1);
                    }
                    else
                    {
                        auto listen_obj = std::find_if(m_lisn_objs.begin(), m_lisn_objs.end(), [=](const LISTEN_OBJ& obj) {
                            return m_Events.at(index) == obj.AcceptEvent || m_Events.at(index) == obj.RepostAccept;
                        });

                        if (listen_obj != m_lisn_objs.end())
                        {
                            WSANETWORKEVENTS ne;
                            int limit = 0;

                            if ((*listen_obj).AcceptEvent == m_Events.at(index))
                            {
                                ret = WSAEnumNetworkEvents((*listen_obj).socket, (*listen_obj).AcceptEvent, &ne);
                                if (ret == SOCKET_ERROR)
                                {
                                    throw wsock_error(WSAGetLastError());
                                }

                                if( (ne.lNetworkEvents & FD_ACCEPT) == FD_ACCEPT )
                                {
                                    // We got an FD_ACCEPT so post multiple accepts to cover the burst
                                    limit = BURST_ACCEPT_COUNT;
                                }
                            }
                            else if ((*listen_obj).RepostAccept == m_Events.at(index))
                            {
                                // Semaphore is signaled
                                limit = (*listen_obj).RepostCount;
                                ResetEvent((*listen_obj).RepostAccept);
                            }
                            int i = 0;
                            BUFFER_OBJ* buffer_obj = nullptr;
                            DWORD bytes = 0;
                            while ((i++ < limit) && ((*listen_obj).PendingAcceptCount < MAX_OVERLAPPED_ACCEPTS))
                            {
                                if (m_StoreBuffer.empty())
                                    buffer_obj = new BUFFER_OBJ;
                                else
                                    buffer_obj = m_StoreBuffer.back();

                                buffer_obj->PostAccept = (*listen_obj).AcceptEvent;

                                (*listen_obj).PendingAccepts.push_back(buffer_obj);

                                buffer_obj->sclient = socket((*listen_obj).AddressFamily, SOCK_STREAM, IPPROTO_TCP);
                                if (buffer_obj->sclient == INVALID_SOCKET)
                                {
                                    throw wsock_error(WSAGetLastError());
                                }

                                BOOL rb = (*listen_obj).lpfnAcceptEx(
                                    (*listen_obj).socket,
                                    buffer_obj->sclient,
                                    buffer_obj->buf,
                                    buffer_obj->buflen - ((sizeof(SOCKADDR_STORAGE) + 16) * 2),
                                    sizeof(SOCKADDR_STORAGE) + 16,
                                    sizeof(SOCKADDR_STORAGE) + 16,
                                    &bytes,
                                    &buffer_obj->ol
                                );

                                if (rb == FALSE)
                                {
                                    if (WSAGetLastError() != WSA_IO_PENDING)
                                    {
                                        throw wsock_error(WSAGetLastError());
                                    }
                                }

                                (*listen_obj).PendingAcceptCount++;
                                //endpointcount++;
                                m_lisn_objs.push_back((*listen_obj));
                            }
                        }
                    }
                }
            }
        }
    }

    void HighPerformanceServer::ProcessEvent()
    {
        DWORD BytesTransfered = 0;
        DWORD Flags = 0;
        ULONG_PTR       Key;
        OVERLAPPED*     lpOverlapped = nullptr;
        BUFFER_OBJ*     buffer_obj = nullptr;
        SOCKET          s = INVALID_SOCKET;
        while (1)
        {
            int ret = GetQueuedCompletionStatus(m_ComPort, &BytesTransfered, (PULONG_PTR)&Key, &lpOverlapped, INFINITE);

            buffer_obj = CONTAINING_RECORD(lpOverlapped, BUFFER_OBJ, ol);

            int error = 0;
            if (ret == FALSE)
            {
                // If the call fails, call WSAGetOverlappedResult to translate the
                //    error code into a Winsock error code.
                if ( buffer_obj->operation == static_cast<int>(OPERATION::OP_ACCEPT) )
                {
                    s = ((LISTEN_OBJ*)Key)->socket;
                }
                else
                {
                    s = ((SOCKET_OBJ*)Key)->socket;
                }
                
                ret = WSAGetOverlappedResult(s, &buffer_obj->ol, &BytesTransfered, FALSE, &Flags);
                if (ret == FALSE)
                {
                    error = WSAGetLastError();
                }
            }

            HandleIO(Key, buffer_obj, m_ComPort, BytesTransfered, error);
        }

        return;
    }

    void HighPerformanceServer::HandleIO(ULONG_PTR key, BUFFER_OBJ* buf, HANDLE CompPort, DWORD BytesTransfered, DWORD error)
    {
        BOOL bCleanupSocket = FALSE;

        LISTEN_OBJ* listen_obj = nullptr;

        if (error != NO_ERROR)
        {
            // An error occurred on a TCP socket, free the associated per I/O buffer
            // and see if there are any more outstanding operations. If so we must
            // wait until they are complete as well.
            if (buf->operation == static_cast<int>(OPERATION::OP_ACCEPT))
            {
                listen_obj = reinterpret_cast<LISTEN_OBJ*>(key);
                Log(LogLevel::ERR, "Accept Failed!");

                closesocket(buf->sclient);
                buf->sclient = INVALID_SOCKET; 
                memset(buf, 0, sizeof(BUFFER_OBJ) + BUFFER_SIZE);
                m_StoreBuffer.push_back(buf);
                return;
            }

            SOCKET_OBJ* sock_obj = reinterpret_cast<SOCKET_OBJ*>(key);
            if ( buf->operation == static_cast<int>(OPERATION::OP_READ) )
            {
                sock_obj->OutstandingRecv--;
                if (sock_obj->OutstandingRecv == 0 && sock_obj->OutstandingSend == 0)
                {
                    Log(LogLevel::INFO, "Freeing socket obj in GetOverlappedResult");

                    if (sock_obj->socket != INVALID_SOCKET)
                    {
                        closesocket(sock_obj->socket);
                        sock_obj->socket = INVALID_SOCKET;
                    }

                    memset(sock_obj, 0, sizeof(SOCKET_OBJ));
                    m_StoreSocketBuffer.push_back(sock_obj);
                }
            }
            else if (buf->operation == static_cast<int>(OPERATION::OP_WRITE))
            {
                sock_obj->OutstandingSend--;
                if (sock_obj->OutstandingRecv == 0 && sock_obj->OutstandingSend == 0)
                {
                    Log(LogLevel::INFO, "Freeing socket obj in GetOverlappedResult");

                    if (sock_obj->socket != INVALID_SOCKET)
                    {
                        closesocket(sock_obj->socket);
                        sock_obj->socket = INVALID_SOCKET;
                    }

                    memset(sock_obj, 0, sizeof(SOCKET_OBJ));
                    m_StoreSocketBuffer.push_back(sock_obj);
                }
            }
            memset(buf, 0, sizeof(BUFFER_OBJ) + BUFFER_SIZE);
            m_StoreBuffer.push_back(buf);
            return;
        }

        if (buf->operation == static_cast<int>(OPERATION::OP_ACCEPT))
        {
            listen_obj = (LISTEN_OBJ*)key;

            SOCKADDR_STORAGE* LocalSockaddr = nullptr;
            SOCKADDR_STORAGE* RemoteSockaddr = nullptr;

            int LocalSockaddrLen = 0; 
            int RemoteSockaddrLen = 0;

            listen_obj->lpfnGetAcceptExSockaddrs(
                buf->buf,
                buf->buflen - ((sizeof(SOCKADDR_STORAGE) + 16) * 2),
                sizeof(SOCKADDR_STORAGE) + 16,
                sizeof(SOCKADDR_STORAGE) + 16,
                (SOCKADDR**)&LocalSockaddr,
                &LocalSockaddrLen,
                (SOCKADDR**)&RemoteSockaddr,
                &RemoteSockaddrLen
            );

            std::erase_if(listen_obj->PendingAccepts, [=](BUFFER_OBJ* buffer) {
                return buffer == buf;
                });

            SOCKET_OBJ* client_obj = nullptr;
            if (!m_StoreSocketBuffer.empty())
            {
                client_obj = m_StoreSocketBuffer.back();
                m_StoreSocketBuffer.pop_back();
            }
            else
            {
                client_obj = new SOCKET_OBJ;
            }

            if (client_obj)
            {
                client_obj->socket = buf->sclient;
                client_obj->af = listen_obj->AddressFamily;

                HANDLE hrc = CreateIoCompletionPort((HANDLE)client_obj->socket, CompPort, (ULONG_PTR)client_obj, 0);
                if (hrc == NULL)
                {
                    throw wsock_error(GetLastError());
                }
                BUFFER_OBJ* send_obj = buf;
                send_obj->buflen = BytesTransfered;
                send_obj->sock_obj = client_obj;
                // PostSend(clientobj, sendobj);
                //EnqueuePendingOperation(&gPendingSendList, &gPendingSendListEnd, sendobj, OP_WRITE);
            }
            else
            {
                closesocket(buf->sclient);
                buf->sclient = INVALID_SOCKET;
                memset(buf, 0, sizeof(BUFFER_OBJ) + BUFFER_SIZE);
                m_StoreBuffer.push_back(buf);
            }
            listen_obj->RepostCount--;
            SetEvent(listen_obj->RepostAccept);
        }
        else if (buf->operation == static_cast<int>(OPERATION::OP_READ))
        {
            SOCKET_OBJ* sock_obj = (SOCKET_OBJ*)key;
            sock_obj->OutstandingRecv--;
            if (BytesTransfered > 0)
            {
                BUFFER_OBJ* send_obj = buf;
                send_obj->buflen = BytesTransfered;
                send_obj->sock_obj = sock_obj;
            }
            else
            {
            }
        }
    }
}
