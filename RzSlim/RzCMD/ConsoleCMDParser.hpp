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
#include <map>

namespace RzLib
{
    class CMD;
    class RzServer;
    /**
     * Parser cmd from console input
     */
    class ConsoleCMDParser
    {
    private:
        const std::map<std::string, CONSOLE_CMD> g_sMapCmd{
            {"select",  CONSOLE_CMD::SELECT},
            {"exit",    CONSOLE_CMD::EXIT},
            {"client",  CONSOLE_CMD::CLIENT},
            {"version", CONSOLE_CMD::VERSION},
            {"ls",      CONSOLE_CMD::LS},
            {"cd",      CONSOLE_CMD::CD},
            {"mkdir",   CONSOLE_CMD::MKDIR},
            {"touch",   CONSOLE_CMD::TOUCH},
            {"rm",      CONSOLE_CMD::REMOVE},
            {"clear",   CONSOLE_CMD::CLEAR}
        };
        const std::map<std::string, FILE_EXTENSION> g_sMapEXT{
            {".exe",    FILE_EXTENSION::EXE},
            {".txt",    FILE_EXTENSION::TXT}
        };
    public:
        ConsoleCMDParser(RzServer* server)
            : m_CMD(CONSOLE_CMD::UNKNOWN) 
            , m_socket{ INVALID_SOCKET }
            , m_Server(server)
        {};
        ConsoleCMDParser(const std::string& CMD, char SPLIT = ' ');
        ~ConsoleCMDParser() {}
        SOCKET          GetSocket()     const { return m_socket; }
        CONSOLE_CMD     GetCMD()        const { return m_CMD; }
        std::string     GetMsg()        const { return m_message; }

        void SetCMD(const std::string& CMD, char SPLIT = ' ');

        void RunCmd();

    private:
        void            Parser(const std::string& CMD, char SPLIT = ' ');
        CONSOLE_CMD     CMD_Cast(const std::string& cmd);
        FILE_EXTENSION  EXT_Cast(const std::string& ext);
    private:
        CONSOLE_CMD     m_CMD;
        SOCKET          m_socket;
        std::string     m_message;
        RzServer*       m_Server;
    };
}
