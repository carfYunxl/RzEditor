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
#include "Core/Core.hpp"

namespace RzLib
{
    /**
     * Parser cmd from console input
     */
    class ConsoleCMDParser
    {
    public:
        ConsoleCMDParser(const std::string& CMD, char SPLIT = ' ');
        ~ConsoleCMDParser() {}
        SOCKET          GetSocket()     const { return m_socket; }
        std::string     GetCMD()        const { return m_CMD; }
        std::string     GetMsg()        const { return m_message; }
        CMDType         GetCmdType()    const { return m_cmdType; }

        void SetCMD(const std::string& CMD, char SPLIT = ' ');

    private:
        void Parser(const std::string& CMD, char SPLIT = ' ');
        const bool IsAllDigits(const std::string& digits) const;
    private:
        std::string         m_CMD;
        SOCKET              m_socket{ INVALID_SOCKET };
        std::string         m_message;
        CMDType             m_cmdType{ CMDType::NONE };
    };
}
