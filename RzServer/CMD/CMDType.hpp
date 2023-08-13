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
        CMD(size_t cmdId, Server* server)
            : m_CmdId(cmdId)
            , m_Server(server){}
        virtual ~CMD() {}
        virtual void Run() = 0;
    protected:
        size_t m_CmdId;
        Server* m_Server;
    };

    struct CMDSingle : public CMD
    {
        CMDSingle(size_t cmdId, Server* server) 
            : CMD(cmdId,server) {}

        virtual ~CMDSingle() {}

        virtual void Run() override;
    };

    struct CMDDouble : public CMD
    {
        CMDDouble(size_t cmdId, Server* server, SOCKET socket) 
            : CMD(cmdId, server)
            , m_socket(socket) {}

        virtual ~CMDDouble() {}

        virtual void Run() override;

    protected:
        SOCKET m_socket;
    };

    struct CMDTriple : public CMDDouble
    {
        CMDTriple(size_t cmdId, Server* server, SOCKET socket, const std::string& msg)
            : CMDDouble(cmdId, server, socket)
            , m_message(msg){}

        virtual ~CMDTriple() {}

        virtual void Run() override;

    protected:
        std::string m_message;
    };
}
