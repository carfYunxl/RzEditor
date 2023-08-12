#pragma once

#include "Server/Server.hpp"

namespace RzLib
{
    struct CMD
    {
        CMD(size_t cmdId, Server* server)
            : m_CmdId(cmdId)
            , m_Server(server){}

        virtual void Run() = 0;
    protected:
        size_t m_CmdId;
        Server* m_Server;
    };

    struct CMDSingle : public CMD
    {
        CMDSingle(size_t cmdId, Server* server) 
            : CMD(cmdId,server) {}

        virtual void Run() override;
    };

    struct CMDDouble : public CMD
    {
        CMDDouble(size_t cmdId, Server* server, SOCKET socket) 
            : CMD(cmdId, server)
            , m_socket(socket) {}

        virtual void Run() override;

    private:
        SOCKET m_socket;
    };

    struct CMDTriple : public CMD
    {
        CMDTriple(size_t cmdId, Server* server, SOCKET socket, const std::string& msg)
            : CMD(cmdId, server)
            , m_socket(socket)
            , m_message(msg){}

        virtual void Run() override;

    private:
        SOCKET m_socket;
        std::string m_message;
    };
}
