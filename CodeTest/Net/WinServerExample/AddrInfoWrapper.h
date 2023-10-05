/*
*    this class collect local ip address for other use.
*/

#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <stdexcept>
#include <string>
#include <string.h>
#include <cassert>

namespace RzLib
{
    // self define exception for winsock2
    class wsock_error : public std::exception
    {
    public:
        using _Mybase = exception;

        explicit wsock_error(int32_t error_code)
            : m_error("winsock2 error with code ")
        {
            m_error.append(std::to_string(error_code));
        }

        explicit wsock_error(const std::string& error)
            : m_error("winsock2 error with code ")
        {
            m_error.append(error);
        }

        explicit wsock_error(const char* error)
            : m_error("winsock2 error with code ")
        {
            m_error.append(error);
        }

        const char* what() const override
        {
            return m_error.c_str();
        }
    private:
        std::string m_error;
    };

    struct AddrInfo
    {
        int             family;
        int             socktype;
        int             protocol;
        sockaddr*       addr;
        size_t          addr_len;
        char            ip[128];
        char            name[128];
    };

    class AddrInfoWrapper
    {
    public:
        AddrInfoWrapper();
        ~AddrInfoWrapper();

        const AddrInfo GetAdrInfo(size_t index) const
        {
            assert(index >= m_AddrInfo.size());

            return m_AddrInfo[index];
        }

        std::vector<AddrInfo> GetAllAdrInfo()
        {
            return m_AddrInfo;
        }

        const std::vector<AddrInfo> GetAllAdrInfo() const
        {
            return m_AddrInfo;
        }
    private:
        void Initialized();
    private:
        std::vector<AddrInfo> m_AddrInfo;
    };
}


