#pragma once

#include <string>
#include <vector>

namespace RzLib
{
    constexpr size_t CMD_MIN_SIZE = 3;

    struct RecvInfo
    {
        unsigned char   cmd;
        std::string     msg;
    };

    class TcpParser
    {
    public:
        TcpParser(const unsigned char* cmd, size_t size);
        ~TcpParser() = default;

        const std::vector<RecvInfo> GetInfo() const { return m_Info; }

        void Set(const unsigned char* cmd) { Parser(cmd); }

    private:
        void Parser(const unsigned char* arrCmd);

        const bool IsCmd(unsigned char ch) const;

    private:
        std::vector<RecvInfo>   m_Info;
        size_t                  m_size;
    };
}
