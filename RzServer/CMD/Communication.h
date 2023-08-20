/*****************************************************************//**
 * \file   Communication.h
 * \brief  a simple class to parser communication cmd
 * 
 *              
 * 
 * \author yxl
 * \date   August 2023
 *********************************************************************/
#pragma once

#include <string>

namespace RzLib
{
    class Communication
    {
    public:
        Communication(const char* bufCmd, int rtLen);

    public:
        const size_t GetCmd() const { return m_Cmd; }
        const std::string GetMsg() const { return m_msg; }
    private:
        void Parser(const char* bufCmd, int rtLen);

    private:
        unsigned char       m_Cmd;
        std::string         m_msg;
    };
}
