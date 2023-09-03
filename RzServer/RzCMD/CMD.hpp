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

    class SendCMD : public CMD
    {
    public:
        SendCMD(CONSOLE_CMD cmd, RzServer* server, SOCKET socket, const std::string& msg)
            : CMD(cmd, server)
            , m_socket(socket)
            , m_message(msg)
        {}
        virtual ~SendCMD() = default;
        virtual void Run() override
        {
            if (std::filesystem::exists(m_message))
            {
                //m_Server->SendFileToClient(m_socket, m_message);
            }
            else
            {
                // 不是路径就是一条信息
                size_t size = m_message.size();
                std::string strSend{
                    static_cast<char>(0xF1),
                    static_cast<char>(size & 0xFF),
                    static_cast<char>((size >> 8) & 0xFF)
                };
                strSend += m_message;

                if (SOCKET_ERROR == send(m_socket, strSend.c_str(), static_cast<int>(strSend.size()), 0))
                {
                    Log(LogLevel::ERR, "send info to client error!");
                }
            }
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
            Log(LogLevel::INFO, "server is closed!");
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
}
