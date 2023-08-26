#pragma once

#include <string>
#include <vector>

namespace RzLib
{
    constexpr size_t CMD_MIN_SIZE = 3;

    enum class RECV_CMD : unsigned char
    {
        NORMAL      = 0xF1,
        VERSION     = 0xF2,
        UPDATE      = 0xF4,
        FILE_HEADER = 0xF5,
        FILE_PACKET = 0xF6,
        File_TAIL   = 0xF7
    };

    struct RecvInfo
    {
        unsigned char   cmd;
        std::string     msg;
    };

    class TcpParser
    {
    public:
        TcpParser(const char* cmd, size_t size);
        ~TcpParser() = default;

        const std::vector<RecvInfo> GetInfo() const { return m_Info; }

        void Set(const char* cmd) { Parser(cmd); }

    private:
        void Parser(const char* arrCmd);

        const bool IsCmd(unsigned char ch) const;

    private:
        std::vector<RecvInfo>   m_Info;
        size_t                  m_size;
    };
}
