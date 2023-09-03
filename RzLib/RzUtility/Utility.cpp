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

    void Utility::PrintConsoleHeader()
    {
        Print(LogLevel::CONSOLE, GetUserInfo() + ":\n");
        Print(LogLevel::CONSOLE, "$ ");
    }

    void Utility::EnSTDOutputColor()
    {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }

    void Utility::ChangeConsoleFont(const std::wstring& faceName)
    {
        CONSOLE_FONT_INFOEX cfi;
        cfi.cbSize = sizeof cfi;
        cfi.nFont = 0;
        cfi.dwFontSize.X = 0;
        cfi.dwFontSize.Y = 15;
        cfi.FontFamily = FF_DONTCARE;
        cfi.FontWeight = FW_NORMAL;
        wcscpy_s(cfi.FaceName, faceName.c_str());
        SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
    }
}