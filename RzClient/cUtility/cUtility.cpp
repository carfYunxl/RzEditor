#include "cUtility/cUtility.hpp"

namespace RzLib
{
    std::string ClientUti::GetUserInfo()
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
    const bool ClientUti::IsAllDigits(const std::string& digits)
    {
        return digits.cend() == std::find_if(digits.cbegin(), digits.cend(), [](const char ch) { return isalpha(ch); });
    }

    void ClientUti::PrintConsoleHeader()
    {
        Print(LogLevel::CONSOLE, GetUserInfo() + ":\n");
        Print(LogLevel::CONSOLE, "$ ");
    }
}