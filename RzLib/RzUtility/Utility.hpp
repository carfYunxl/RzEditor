#pragma once
#include <string>

namespace RzLib
{
    class Utility
    {
    public:
        static std::string GetUserInfo();

        static const bool IsAllDigits(const std::string& digits);

        static void PrintConsoleHeader(const std::string& path);
    };
}
