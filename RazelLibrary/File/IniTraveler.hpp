/*****************************************************************//**
 * @file   IniTraveler.hpp
 * @brief  A ini file parser class
 * 
 * @author yxl
 * @date   July 2023
 *********************************************************************/

#pragma once

#include "FileTraveler.hpp"

namespace RzLib
{
    class IniTraveler : public FileTraveler
    {
    public:
        explicit IniTraveler(std::string&& filepath);
        explicit IniTraveler(const std::string& filepath);

        virtual bool open() override;

    private:

    };
}