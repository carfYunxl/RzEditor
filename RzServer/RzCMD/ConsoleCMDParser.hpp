/*****************************************************************//**
 * \file   CMDParser.hpp
 * \brief  Several classes to handle cmd input from console.
 *              - handle server command
 *              - handle client command
 * 
 * \author yun
 * \date   August 2023
 *********************************************************************/
#pragma once
#include <string>
#include <winsock2.h>
#include "RzCore/Core.hpp"
#include <memory>

namespace RzLib
{
    class CMD;
    class RzServer;
    /**
     * Parser cmd from console input
     */
    class ConsoleCMDParser
    {
    public:
        ConsoleCMDParser() : m_CMD(CONSOLE_CMD::UNKNOWN) {};
        ConsoleCMDParser(const std::string& CMD, char SPLIT = ' ');
        ~ConsoleCMDParser() {}
        SOCKET          GetSocket()     const { return m_socket; }
        CONSOLE_CMD     GetCMD()        const { return m_CMD; }
        std::string     GetMsg()        const { return m_message; }
        CMDType         GetCmdType()    const { return m_cmdType; }

        void SetCMD(const std::string& CMD, char SPLIT = ' ');

        std::unique_ptr<CMD> GenCmd(RzServer* server);

        template<typename TYPECMD>
        void RunCmd(TYPECMD pCMD)
        {
            pCMD->Run();
        }
    private:
        void Parser(const std::string& CMD, char SPLIT = ' ');
        const bool IsCmd(const std::string& sCmd);
        const bool IsFunCMD(const std::string& fCMD);
        const bool IsTransCMD(const std::string& tCMD);

        CONSOLE_CMD CastCMD(const std::string& cmd);

    private:
        CONSOLE_CMD     m_CMD;
        SOCKET          m_socket{ INVALID_SOCKET };
        std::string     m_message;
        CMDType         m_cmdType{ CMDType::NONE };
    };
}
