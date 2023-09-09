#include "pch.h"
#include "RzUtility/Utility.hpp"
#include "RzCore/Log.hpp"
#include "RzCore/Core.hpp"

namespace RzLib
{
    std::string Utility::GetUserInfo()
    {
        std::string str;
        TCHAR szBuffer[64]{ 0 };
        DWORD len = sizeof(szBuffer);

        if (GetUserName(szBuffer, &len))
        {
            USES_CONVERSION;
            str += std::string(T2A(szBuffer));
        }
        memset(szBuffer, 0, 64);
        len = sizeof(szBuffer);
        if (GetComputerName(szBuffer, &len))
        {
            USES_CONVERSION;
            str += "@" + std::string(T2A(szBuffer));
        }

        return str;
    }
    const bool Utility::IsAllDigits(const std::string& digits)
    {
        return digits.cend() == std::find_if(digits.cbegin(), digits.cend(), [](const char ch) { return isalpha(ch); });
    }

    void Utility::PrintConsoleHeader(const std::string& path)
    {
        Print(LogLevel::CONSOLE, GetUserInfo() + ": ");
        Print(LogLevel::WARN, path + "\n");
        Print(LogLevel::INFO, "$ ");
    }
}