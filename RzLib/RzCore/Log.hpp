#pragma once

#include <type_traits>
#include <iostream>

#define FMT_HEADER_ONLY

#include "FmtLayer.hpp"
#include "RzUtility/Utility.hpp"

namespace RzLib
{
    enum class LogLevel
    {
        INFO = 0,
        WARN,
        ERR,
        CONSOLE
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
            fmt::print(fg(fmt::color::light_pink), "[INFO] ");
            break;
        case LogLevel::WARN:
            fmt::print(fg(fmt::color::yellow), "[WARN] ");
            break;
        case LogLevel::ERR:
            fmt::print(fg(fmt::color::red), "[ERROR] ");
            break;
        case LogLevel::CONSOLE:
            fmt::print(fg(fmt::color::yellow), "[CONSOLE] ");
            break;
        }

        (std::cout << ... << AddSeperator<Args,' '>(args)) << std::endl;

        Utility::PrintConsoleHeader();
    }

    template<typename LogLevel>
    void Print(LogLevel level, const std::string& title)
    {
        switch (level)
        {
            case LogLevel::INFO:
                fmt::print(fg(fmt::color::light_pink), title);
                break;
            case LogLevel::WARN:
                fmt::print(fg(fmt::color::yellow), title);
                break;
            case LogLevel::ERR:
                fmt::print(fg(fmt::color::red), title);
                break;
            case LogLevel::CONSOLE:
                fmt::print(fg(fmt::color::yellow), title);
                break;
        }
    }
}