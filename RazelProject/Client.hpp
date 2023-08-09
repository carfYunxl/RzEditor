#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <thread>
#include <string>

void p()
{
    WSADATA data;
    if (WSAStartup(MAKEWORD(2, 2), &data) < 0)
    {
        //...failed!
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //iMode = 0, blocking is enabled
    //iMode != 0, non-blocking is enabled
    //u_long iMode = 1;

    //int nRet = ioctlsocket(clientSocket, FIONBIO, &iMode);

    SOCKADDR_IN serverAddr;

    const int PORT = 8080;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "192.168.2.12", &serverAddr.sin_addr);

    if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        //...failed!
    }

    //fd_set set;

    //FD_ZERO(&set);

    //FD_CLR(clientSocket, &set);

    //FD_ISSET(clientSocket, &set);

    //FD_SET(clientSocket, &set);

    std::thread thread([=]() {
        char recvBuf[1500]{ 0 };
        if (recv(clientSocket, recvBuf, 1500, 0) > 0)
        {
            std::cout << "Recv from server : " << recvBuf << std::endl;
        }
        });

    thread.detach();

    //发送数据、接收数据
    std::string str;
    while (std::cin >> str)
    {
        if (str == "quit")
        {
            break;
        }

        std::cout << "send to server:" << std::endl;
        if (send(clientSocket, &str[0], str.size(), 0) > 0)
        {
            std::cout << "send success!:" << std::endl;
        }

        str.clear();
    }


    shutdown(clientSocket, SD_BOTH);
    closesocket(clientSocket);

    WSACleanup();
}
