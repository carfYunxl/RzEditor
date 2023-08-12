#pragma once
#include <string>
#include <winsock2.h>
#include "Core/Core.hpp"

namespace RzLib
{
    struct CMDInfo
    {
        size_t CMD;
        SOCKET socket;
        std::string message;
    };

    enum class CMDType
    {
        NONE = 0,
        SINGLE,
        DOUBLE,
        TRIPLE
    };

    class CMDParser
    {
    public:
        CMDParser(const std::string& CMD, char SPLIT = ' ');

        SOCKET GetSocket()      const { return m_cmdInfo.socket; }
        size_t GetCMD()         const { return m_cmdInfo.CMD; }
        std::string GetMsg()    const { return m_cmdInfo.message; }

        void SetCMD(const std::string& CMD, char SPLIT = ' ');

        CMDType GetCmdType() const { return m_cmdType; }

    private:
        void Parser(const std::string& CMD, char SPLIT = ' ');
        bool IsAllDigits(const std::string& digits);
    private:
        CMDInfo m_cmdInfo;
        CMDType m_cmdType{ CMDType::NONE };
    };
}
