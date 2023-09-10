/*****************************************************************//**
 * \file   Communication.h
 * \brief  a simple class to parser cmd message send from client, like below
 * 
 *              1> 0xF1 0x00 0x00 ...
 *              2> 0xF3 0x00 0x00 ...    
 * 
 * \author yxl
 * \date   August 2023
 *********************************************************************/
#pragma once

#include <string>
#include <winsock2.h>

namespace RzLib
{
    class RzServer;
    class TcpCmdParser
    {
    public:
        TcpCmdParser(RzServer* server, SOCKET socket, const char* bufCmd, int rtLen);

    public:
        const size_t        GetCmd() const { return m_Cmd; }
        const std::string   GetMsg() const { return m_msg; }

        void RunCmd();
    private:
        void        Parser(const char* bufCmd, int rtLen);
        const bool  IsCmd() const;
        bool        SendFileToClient(const std::string& path);
    private:
        SOCKET              m_socket;
        unsigned char       m_Cmd;
        std::string         m_msg;
        RzServer*           m_server;
    };
}
