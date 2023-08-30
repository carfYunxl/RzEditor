#include "pch.h"
#include "RzFile/FileTraveler.hpp"
#include <filesystem>
#include <fstream>

namespace RzLib
{
    /** move constructor */
    FileTraveler::FileTraveler(std::string&& filepath)
        : m_filepath(std::move(filepath))
    {
        Log(LogLevel::INFO, "call move ctor!");
    }

    /** reference constructor */
    FileTraveler::FileTraveler(const std::string& filepath)
        : m_filepath(filepath)
    {
        Log(LogLevel::WARN, "call normal ctor!");
    }

    FileTraveler& FileTraveler::operator=(const FileTraveler& rhs)
    {
        m_fileCache = rhs.m_fileCache;
        return *this;
    }
    FileTraveler& FileTraveler::operator=(FileTraveler&& rhs) noexcept
    {
        m_fileCache = std::move(rhs.m_fileCache);
        return *this;
    }

    /** open file */
    bool FileTraveler::open(Mode mode)
    {
        if (!std::filesystem::exists(m_filepath))
        {
            std::ofstream out;
            if (mode == Mode::Binary)
            {
                out.open(m_filepath, std::ofstream::out | std::ofstream::binary);
            }
            else
                out.open(m_filepath, std::ofstream::out);

            if (!out.is_open())
            {
                Log(LogLevel::ERR, "Couldn't create file : ", m_filepath);
                return false;
            }
            out.close();
        }

        std::ifstream inf;
        if(mode == Mode::Binary)
            inf.open(m_filepath, std::ofstream::binary);
        else
            inf.open(m_filepath, std::fstream::in);
        if (!inf.is_open())
        {
            Log(LogLevel::ERR, "Couldn't open file : ", m_filepath);
            return false;
        }

        m_fileCache.clear();
        inf.seekg(0, std::ios::end);
        std::size_t size = inf.tellg();
        if (size == -1)
        {
            Log(LogLevel::ERR, "Couldn't read from file : ", m_filepath);
            return false;
        }

        m_fileCache.resize(size);
        inf.seekg(0, std::ios::beg);

        inf.read(&m_fileCache[0], size);
        inf.close();

        Log(LogLevel::INFO, "Open file success : ", m_filepath);
        return true;
    }

    /** append text to file end */
    void FileTraveler::append(const char* text)
    {
        m_fileCache.append(text);
    }

    /** close file */
    void FileTraveler::close()
    {
        std::ofstream out(m_filepath, std::ofstream::binary);
        if (!out.is_open())
        {
            Log(LogLevel::ERR,"Couldn't create file : ", m_filepath);
            return;
        }

        out.write(&m_fileCache[0],m_fileCache.size());

        Log(LogLevel::WARN, "write file size = ", m_fileCache.size());

        out.flush();
        out.close();

        Log(LogLevel::INFO, "save file : ", m_filepath, "success!");
        return;
    }

    /** get file content */
    const std::string FileTraveler::GetFileContent() const
    {
        return m_fileCache;
    }

    /** replace key as text, count means times you want to replace */
    void FileTraveler::replace(const char* key, const char* text, size_t count)
    {
        size_t keyIndex = m_fileCache.find(key, 0);
        while (keyIndex != std::string::npos && count > 0)
        {
            m_fileCache.replace(keyIndex, strlen(key), text);
            keyIndex = m_fileCache.find(key, keyIndex);
            count--;
        }
    }

    /** replace all keys as text */
    void FileTraveler::replace_all(const char* key, const char* text)
    {
        size_t keyIndex = m_fileCache.find(key, 0);
        while (keyIndex != std::string::npos)
        {
            m_fileCache.replace( keyIndex, strlen(key), text);
            keyIndex = m_fileCache.find(key, keyIndex);
        }
    }

    /** replace first key as text */
    void FileTraveler::replace_the_first_of(const char* key, const char* text)
    {
        size_t keyIndex = m_fileCache.find(key);
        if (keyIndex != std::string::npos)
        {
            m_fileCache.replace( keyIndex, strlen(key), text);
        }
    }

    /** replace last key as text */
    void FileTraveler::replace_the_last_of(const char* key, const char* text)
    {
        size_t keyIndex = m_fileCache.rfind(key);
        if (keyIndex != std::string::npos)
        {
            m_fileCache.replace(keyIndex, strlen(key), text);
        }
    }

    /** replace key from file end as text. count means key number you want to repalce */
    void FileTraveler::replace_reverse(const char* key, const char* text, size_t count)
    {
        size_t keyIndex = m_fileCache.rfind(key, 0);
        while (keyIndex != std::string::npos && count > 0)
        {
            m_fileCache.replace(keyIndex, strlen(key), text);
            keyIndex = m_fileCache.rfind(key, keyIndex);
            count--;
        }
    }

    /** insert text at the front of the key */
    void FileTraveler::insert_front(const char* key, const char* text)
    {
        size_t keyIndex = m_fileCache.find(key);
        if (keyIndex != std::string::npos)
        {
            m_fileCache.insert( keyIndex, text);
        }
    }

    /** insert text at the end of the key */
    void FileTraveler::insert_after(const char* key, const char* text)
    {
        size_t keyIndex = m_fileCache.find(key);
        if (keyIndex != std::string::npos)
        {
            m_fileCache.insert(keyIndex+strlen(key), text);
        }
    }

    /** 交换两个文件的内容，实际上是交换文件的名字 */
    void FileTraveler::swap(FileTraveler& rhs)
    {
        std::swap(m_filepath, rhs.m_filepath);
    }
}