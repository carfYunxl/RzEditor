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
    struct CMDInfo
    {
        size_t CMD{0};
        SOCKET socket{INVALID_SOCKET};
        std::string message;
    };

    enum class CMDType
    {
        NONE = 0,
        SINGLE,
        DOUBLE,
        TRIPLE
    };

    class CMDParserClient
    {
    public:
        CMDParserClient(const std::string& CMD, char SPLIT = ' ');
        virtual ~CMDParserClient() {}

        virtual void SetCMD(const std::string& CMD, char SPLIT = ' ');
        size_t GetCMD() const { return m_CMD; }

    protected:
        virtual void Parser(const std::string& CMD, char SPLIT = ' ');
    protected:
        size_t m_CMD;
    };

    class CMDParser : public CMDParserClient
    {
    public:
        CMDParser(const std::string& CMD, char SPLIT = ' ');
        ~CMDParser() {}
        SOCKET      GetSocket()     const { return m_socket; }
        size_t      GetCMD()        const { return m_CMD; }
        std::string GetMsg()        const { return m_message; }
        CMDType     GetCmdType()    const { return m_cmdType; }

        virtual void SetCMD(const std::string& CMD, char SPLIT = ' ') override;

    private:
        virtual void Parser(const std::string& CMD, char SPLIT = ' ') override;
        bool IsAllDigits(const std::string& digits);
    private:
        SOCKET      m_socket{ INVALID_SOCKET };
        std::string m_message;
        CMDType     m_cmdType{ CMDType::NONE };
    };
}
