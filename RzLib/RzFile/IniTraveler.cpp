#include "pch.h"
#include "IniTraveler.hpp"

namespace RzLib
{
    IniTraveler::IniTraveler(std::string&& filepath)
        :FileTraveler(filepath)
    {}
    IniTraveler::IniTraveler(const std::string& filepath)
        :FileTraveler(filepath)
    {}

    bool IniTraveler::open(Mode mode)
    {
        if (!FileTraveler::open(mode))
        {
            return false;
        }

        // 搜集所有的section并保存起来
        parser_sections();
        return true;
    }

    void IniTraveler::parser_sections()
    {
        std::string_view cache_str(m_fileCache);
        size_t left = cache_str.find("[");
        size_t right = cache_str.find("]");
        while (left != std::string::npos && right != std::string::npos)
        {
            m_vecSections.emplace_back(cache_str.substr(left, right-left+1));

            left = cache_str.find("[");
            right = cache_str.find("]");
        }
    }

    bool IniTraveler::section_exists(const std::string& section)
    {
        return std::find(m_vecSections.begin(),m_vecSections.end(),section) != m_vecSections.end();
    }
}
