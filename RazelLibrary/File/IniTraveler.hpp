/*****************************************************************//**
 * @file   IniTraveler.hpp
 * @brief  A ini file parser class
 * 
 * @author yxl
 * @date   July 2023
 *********************************************************************/

#pragma once

#pragma warning(disable:4251)

#include "FileTraveler.hpp"
#include <vector>

namespace RzLib
{
    class RzAPI IniTraveler : public FileTraveler
    {
    public:
        explicit IniTraveler(std::string&& filepath);
        explicit IniTraveler(const std::string& filepath);

        virtual bool open() override;

        bool section_exists(const std::string& section);
    private:
        void parser_sections();
    private:
        std::vector<std::string> m_vecSections;
    };
}