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

#include "RzClient/RzClient.hpp"

namespace RzLib
{
    struct CMD
    {
        CMD(const std::string& cmd, RzClient* client)
        : m_Cmd(cmd), m_Client(client){}
        virtual ~CMD() {}
        virtual void Run() = 0;
    protected:
        std::string     m_Cmd;
        RzClient*       m_Client;
    };

    struct CMDSingle : public CMD
    {
        CMDSingle(const std::string& cmd, RzClient* client)
            : CMD(cmd,client) {}

        virtual ~CMDSingle() {}

        virtual void Run() override;
    };

    struct CMDDouble : public CMD
    {
        CMDDouble(const std::string& cmd, RzClient* client, const std::string& info)
            : CMD(cmd, client)
            , m_SecInfo(info) {}

        virtual ~CMDDouble() {}

        virtual void Run() override;

    protected:
        std::string m_SecInfo;;
    };

    struct CMDTriple : public CMDDouble
    {
        CMDTriple(const std::string& cmd, RzClient* client, const std::string& info, const std::string& msg)
            : CMDDouble(cmd, client, info)
            , m_message(msg){}

        virtual ~CMDTriple() {}

        virtual void Run() override;

    protected:
        std::string m_message;
    };
}
