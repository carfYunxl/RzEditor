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
    //0. ��ʼ��winsock2
    WSADATA sData;
    if ( WSAStartup(MAKEWORD(2,2),&sData) != 0 )
    {
        return -1;
    }

    //1.����һ����ɶ˿�
    HANDLE hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    //2.ȷ��ϵͳ���ж��ٸ�������

    return 0;
}
