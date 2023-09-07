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
#include "RzCore/Core.hpp"
#include "RzServer/RzServer.hpp"
#include "RzCore/Log.hpp"

namespace RzLib
{
    //=== Console Imput Class ========================================================
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

    class SelectCMD : public CMD
    {
    public:
        SelectCMD(CONSOLE_CMD cmd, RzServer* server, SOCKET socket, const std::string& msg)
            : CMD(cmd, server)
            , m_socket(socket)
            , m_message(msg)
        {}
        virtual ~SelectCMD() = default;
        virtual void Run() override
        {
            m_Server->SetInputMode(InputMode::SEND);
            m_Server->SelectClient(m_socket);
        }

    private:
        SOCKET          m_socket;
        std::string     m_message;
    };

    struct ExitCMD : public CMD
    {
        ExitCMD(CONSOLE_CMD cmd, RzServer* server) : CMD(cmd,server) {}
        virtual void Run() override
        {
            m_Server->StopServer();
        }
    };

    struct ClientCMD : public CMD
    {
        ClientCMD(CONSOLE_CMD cmd, RzServer* server) : CMD(cmd, server) {}
        virtual void Run() override
        {
            m_Server->ListClient();
        }
    };

    struct VersionCMD : public CMD
    {
        VersionCMD(CONSOLE_CMD cmd, RzServer* server) : CMD(cmd, server) {}
        virtual void Run() override
        {
            // ... TODO
        }
    };

    struct LsCMD : public CMD
    {
        LsCMD(CONSOLE_CMD cmd, RzServer* server) : CMD(cmd, server) {}
        virtual void Run() override
        {
            // ... TODO
        }
    };

    struct CdCMD : public CMD
    {
        CdCMD(CONSOLE_CMD cmd, RzServer* server) : CMD(cmd, server) {}
        virtual void Run() override
        {
            // ... TODO
        }
    };
}
