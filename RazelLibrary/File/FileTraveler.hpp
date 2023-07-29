#pragma once

#include "Core/Core.hpp"
#include "Core/Log.hpp"
#include <string>

namespace RzLib
{
    class RazelAPI FileTraveler
    {
    public:
        explicit FileTraveler(std::string&& filepath);
        explicit FileTraveler(const std::string& filepath);

        FileTraveler(const FileTraveler& rhs) = default;
        FileTraveler(FileTraveler&& rhs) noexcept = default;

        virtual ~FileTraveler() = default;

        FileTraveler& operator=(const FileTraveler& rhs);
        FileTraveler& operator=(FileTraveler&& rhs) noexcept;

        virtual bool open();

        virtual void replace                ( const char* key, const char* text, size_t count = 1);
        virtual void replace_all            ( const char* key, const char* text );
        virtual void replace_the_first_of   ( const char* key, const char* text );
        virtual void replace_the_last_of    ( const char* key, const char* text );
        virtual void replace_reverse        ( const char* key, const char* text, size_t count = 1);

        virtual void insert_front(const char* key, const char* text);
        virtual void insert_after(const char* key, const char* text);

        virtual void append(const char* text);
        virtual void close();

        virtual void swap(FileTraveler& rhs);

        const std::string GetFileContent() const;
    private:
        std::string m_filepath;
        std::string m_fileCache;
    };

}