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
        CMD(CONSOLE_CMD cmd, RzServer* server);

        virtual void Run() {};
        virtual ~CMD() {}

    protected:
        CONSOLE_CMD     m_Cmd;
        RzServer*       m_Server;
    };

    /** exit, client, version ... not used for transfer information */
    class FuncCMD : public CMD
    {
    public:
        FuncCMD(CONSOLE_CMD cmd, RzServer* server);

        virtual ~FuncCMD() {}

        virtual void Run() override;
    };

    /** used for transfor information  */
    class TransferCMD : public CMD
    {
    public:
        TransferCMD(CONSOLE_CMD cmd, RzServer* server, SOCKET socket, const std::string& msg);

        virtual ~TransferCMD() {}

        virtual void Run() override;

    protected:
        SOCKET          m_socket;
        std::string     m_message;
    };
}
