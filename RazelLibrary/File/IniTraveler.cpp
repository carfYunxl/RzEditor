#include "IniTraveler.hpp"

namespace RzLib
{
    IniTraveler::IniTraveler(std::string&& filepath)
        :FileTraveler(filepath)
    {}
    IniTraveler::IniTraveler(const std::string& filepath)
        :FileTraveler(filepath)
    {}

    bool IniTraveler::open()
    {
        if (!FileTraveler::open())
        {
            return false;
        }

        // 搜集所有的section并保存起来
    }
}
