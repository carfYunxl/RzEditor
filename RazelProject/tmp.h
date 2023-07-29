#pragma once

#include <string>
#include <iostream>

class File
{
public:
    template<typename STR>
    explicit File(STR&& filepath) : m_filepath(std::forward<STR>(filepath))
    {
        
        std::cout << "call tmp ctor!" << std::endl;
    
    }

    File(const File& rhs) = default;
    File(File&& rhs) noexcept = default;

    ~File() = default;

    File& operator=(const File& rhs) = default;
    File& operator=(File&& rhs) noexcept = default;

    const std::string GetFileContent() const { return m_filepath; }
private:
    std::string m_filepath;
};