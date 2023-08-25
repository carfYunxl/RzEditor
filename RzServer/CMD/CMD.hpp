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

#include <string>
#include <winsock2.h>

class RzServer;

namespace RzLib
{
    class CMD
    {
    public:
        CMD(const std::string& cmd, RzServer* server);

        virtual void Run() {};
        virtual ~CMD() {}

    protected:
        std::string     m_Cmd;
        RzServer*       m_Server;
    };

    class CMDSingle : public CMD
    {
    public:
        CMDSingle(const std::string& cmd, RzServer* server);

        virtual ~CMDSingle() {}

        virtual void Run() override;
    };

    class CMDDouble : public CMD
    {
    public:
        CMDDouble(const std::string& cmd, RzServer* server, SOCKET socket);

        virtual ~CMDDouble() {}

        virtual void Run() override;

    protected:
        SOCKET m_socket;
    };

    class CMDTriple : public CMD
    {
    public:
        CMDTriple(const std::string& cmd, RzServer* server, SOCKET socket, const std::string& msg);

        virtual ~CMDTriple() {}

        virtual void Run() override;

    protected:
        SOCKET m_socket;
        std::string m_message;
    };
}
