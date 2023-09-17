#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

struct PER_HANDLE_DATA
{
    SOCKET sSocket;
    SOCKADDR_STORAGE ClientAddr;
};

int main()
{
    //0. 初始化winsock2
    WSADATA sData;
    if ( WSAStartup(MAKEWORD(2,2),&sData) != 0 )
    {
        return -1;
    }

    //1.创建一个完成端口
    HANDLE hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    //2.确定系统中有多少个处理器

    return 0;
}
