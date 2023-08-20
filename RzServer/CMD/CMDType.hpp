/*****************************************************************//**
 * \file   CMDType.hpp
 * \brief  Several classed about cmd type
 *          - Single Command : 
 *          - Double Command :
 *          - Triple Command :
 * 
 * \author yun
 * \date   August 2023
 *********************************************************************/

#pragma once

#include "Server/Server.hpp"

namespace RzLib
{
    struct CMD
    {
        CMD(const std::string& cmd, Server* server)
            : m_Cmd(cmd)
            , m_Server(server){}
        virtual ~CMD() {}
        virtual void Run() = 0;
    protected:
        std::string m_Cmd;
        Server* m_Server;
    };

    struct CMDSingle : public CMD
    {
        CMDSingle(const std::string& cmd, Server* server)
            : CMD(cmd,server) {}

        virtual ~CMDSingle() {}

        virtual void Run() override;
    };

    struct CMDDouble : public CMD
    {
        CMDDouble(const std::string& cmd, Server* server, SOCKET socket)
            : CMD(cmd, server)
            , m_socket(socket) {}

        virtual ~CMDDouble() {}

        virtual void Run() override;

    protected:
        SOCKET m_socket;
    };

    struct CMDTriple : public CMDDouble
    {
        CMDTriple(const std::string& cmd, Server* server, SOCKET socket, const std::string& msg)
            : CMDDouble(cmd, server, socket)
            , m_message(msg){}

        virtual ~CMDTriple() {}

        virtual void Run() override;

    protected:
        std::string m_message;
    };
}
