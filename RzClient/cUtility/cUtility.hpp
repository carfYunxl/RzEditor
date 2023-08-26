#pragma once
#include <string>
#include "Core/Log.hpp"
#include "Core/Core.hpp"
#include <windows.h>
#include <atlconv.h>

namespace RzLib
{
    class ClientUti
    {
    public:
        static std::string GetUserInfo();

        static const bool IsAllDigits(const std::string& digits);

        static void PrintConsoleHeader();
    };
}