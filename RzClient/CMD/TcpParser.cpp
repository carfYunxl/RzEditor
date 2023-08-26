#include "TcpParser.h"
#include "Core/Log.hpp"
#include <algorithm>
#include <numeric>

namespace RzLib
{
    TcpParser::TcpParser(const char* cmd, size_t size)
        : m_size(size)
    {
        Parser(cmd);
    }

    void TcpParser::Parser(const char* arrCmd)
    {
        if (m_size == 0)
            return;

        std::string strRead;
        strRead.resize(m_size);

        memcpy(&strRead[0], arrCmd, m_size);

        unsigned char cmd = static_cast<unsigned char>(strRead[0]);
        std::string strMsg;
        while ( IsCmd(cmd) )
        {
            int nMsgSize = strRead[1] | (strRead[2] << 8);

            if (nMsgSize != 0)
            {
                strMsg.resize(nMsgSize);
                memcpy(&strMsg[0], &strRead[3], nMsgSize);
            }
            
            m_Info.emplace_back(cmd, strMsg);

            strMsg.clear();         

            strRead = strRead.substr( 3 + nMsgSize, strRead.size() - 3 - nMsgSize);

            if (strRead.empty())
                break;

            cmd = static_cast<unsigned char>(strRead[0]);
        }
    }

    const bool TcpParser::IsCmd(unsigned char ch) const
    {
        if (
            ch == 0xF1 ||
            ch == 0xF2 ||
            ch == 0xF4 ||
            ch == 0xF5 ||
            ch == 0xF6 ||
            ch == 0xF7
            )
        {
            return true;
        }

        return false;
    }
}
