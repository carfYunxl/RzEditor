#pragma once

#define FMT_HEADER_ONLY

#include "FmtLayer.hpp"
#include <type_traits>
#include <iostream>

namespace RzLib
{
    enum class LogLevel
    {
        INFO = 0,
        WARN,
        ERR
    };

    template<typename T, char SEPERATOR>
    class AddSeperator
    {
    public:
        AddSeperator(const T& ref) : m_ref(ref) {}

        friend std::ostream& operator<<(std::ostream& out, const AddSeperator& sep)
        {
            out << sep.m_ref << SEPERATOR;
            return out;
        }
    private:
        const T& m_ref;
    };

    template<typename LogLevel, typename...Args>
    void Log(LogLevel level, Args...args)
    {
        switch (level)
        {
        case LogLevel::INFO:
            fmt::print(fg(fmt::color::steel_blue), "[INFO] ");
            break;
        case LogLevel::WARN:
            fmt::print(fg(fmt::color::yellow), "[WARN] ");
            break;
        case LogLevel::ERR:
            fmt::print(fg(fmt::color::red), "[ERROR] ");
            break;
        }

        (std::cout << ... << AddSeperator<Args,' '>(args)) << std::endl;
    }
}