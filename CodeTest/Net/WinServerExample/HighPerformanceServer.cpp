#include "HighPerformanceServer.h"
#include "RzCore/Log.hpp"

namespace RzLib
{
    bool HighPerformanceServer::Init()
    {
        WSADATA sData;
        if (WSAStartup(MAKEWORD(2, 2), &sData) != 0)
        {
            Log(LogLevel::ERR, "fail to load Winsock!");
            return false;
        }

        HANDLE CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)NULL, 0);
        if (CompletionPort == NULL)
        {
            Log(LogLevel::ERR, "CreateIoCompletionPort failed : ", GetLastError());
            return false;
        }

        SYSTEM_INFO sInfo;
        GetSystemInfo(&sInfo);
        if (sInfo.dwNumberOfProcessors > MAX_COMPLETION_THREAD_COUNT)
        {
            sInfo.dwNumberOfProcessors = MAX_COMPLETION_THREAD_COUNT;
        }

        // Round the buffer size to the next increment of the page size
        if ((mBufferSize % sInfo.dwPageSize) != 0)
        {
            mBufferSize = ((mBufferSize / sInfo.dwPageSize) + 1) * sInfo.dwPageSize;
        }
        Log(LogLevel::INFO, "Buffer size = ", mBufferSize, "page size = ", sInfo.dwPageSize);
    }
}
