/*****************************************************************//**
 * \file   CMDType.hpp
 * \brief  Several classed about cmd type
 *          - Single Command : 
 *          - Double Command :
 *          - Triple Command :
 * 
 * \author yun
 * \date   August 2023
 *********************************************************************/

#pragma once

#include <string>
#include <winsock2.h>
#include "RzCore/Core.hpp"
#include "RzCore/Log.hpp"
#include "RzServer/RzServer.hpp"
#include <filesystem>

namespace RzLib
{
    //=== Console Imput Class ========================================================
    class CMD
    {
    public:
        CMD(CONSOLE_CMD cmd, RzServer* server);

        virtual void Run() {};
        virtual ~CMD() {}

    protected:
        CONSOLE_CMD     m_Cmd;
        RzServer*       m_Server;
    };

    class SelectCMD : public CMD
    {
    public:
        SelectCMD(CONSOLE_CMD cmd, RzServer* server, SOCKET socket, const std::string& msg)
            : CMD(cmd, server)
            , m_socket(socket)
            , m_message(msg)
        {}
        virtual ~SelectCMD() = default;
        virtual void Run() override
        {
            m_Server->SetInputMode(InputMode::SEND);
            m_Server->SelectClient(m_socket);
        }

    private:
        SOCKET          m_socket;
        std::string     m_message;
    };

    struct ExitCMD : public CMD
    {
        ExitCMD(CONSOLE_CMD cmd, RzServer* server) : CMD(cmd,server) {}
        virtual void Run() override
        {
            m_Server->StopServer();
        }
    };

    struct ClientCMD : public CMD
    {
        ClientCMD(CONSOLE_CMD cmd, RzServer* server) : CMD(cmd, server) {}
        virtual void Run() override
        {
            m_Server->ListClient();
        }
    };

    struct VersionCMD : public CMD
    {
        VersionCMD(CONSOLE_CMD cmd, RzServer* server) : CMD(cmd, server) {}
        virtual void Run() override
        {
            // ... TODO
        }
    };

    struct LsCMD : public CMD
    {
        LsCMD(CONSOLE_CMD cmd, RzServer* server) : CMD(cmd, server) {}
        virtual void Run() override
        {
            // 列出当前目录下的所有文件名
            for (const auto& dir_entry : std::filesystem::directory_iterator{ m_Server->GetCurrentDir() })
            {
                m_Server->GetUI()->Log_NextLine(LogLevel::INFO, QString("\t%1").arg(dir_entry.path().filename().string().c_str()));
            }
        }
    };

    struct CdCMD : public CMD
    {
        CdCMD(CONSOLE_CMD cmd, RzServer* server, const std::string& path)
            : CMD(cmd, server)
            , m_path(path) {}

        virtual void Run() override
        {
            if (m_path == ".")
            {
                for (const auto& dir_entry : std::filesystem::directory_iterator{ m_Server->GetCurrentDir() })
                {
                    m_Server->GetUI()->Log_NextLine(LogLevel::INFO, QString("\t%1").arg(dir_entry.path().filename().string().c_str()));
                }
                return;
            }
            else if (m_path == "..")
            {
                m_Server->SetCurrentDir(m_Server->GetCurrentDir().parent_path());
            }
            else
            {
                std::filesystem::path path = m_Server->GetCurrentDir() / m_path;
                if (std::filesystem::is_directory(path))
                {
                    m_Server->SetCurrentDir(path);
                }
                else
                {
                    m_Server->GetUI()->Log_NextLine( LogLevel::ERR, "directory not exist!" );
                }
            }
        }

    private:
        std::string m_path;
    };

    struct MkdirCMD : public CMD
    {
        MkdirCMD(CONSOLE_CMD cmd, RzServer* server, const std::string& path)
            : CMD(cmd, server)
            , m_DirName(path) {}

        virtual void Run() override
        {
            if (std::filesystem::is_directory(m_DirName))
            {
                m_Server->SetCurrentDir(m_DirName);
            }
            else
            {
                std::filesystem::path path = m_Server->GetCurrentDir() / m_DirName;
                if (!std::filesystem::exists(path) && !path.has_extension())
                {
                    std::filesystem::create_directories(path);
                }
            }
        }

    private:
        std::string m_DirName;
    };

    struct TouchCMD : public CMD
    {
        TouchCMD(CONSOLE_CMD cmd, RzServer* server, const std::string& path)
            : CMD(cmd, server)
            , m_FileName(path) {}

        virtual void Run() override
        {
            std::filesystem::path path = m_Server->GetCurrentDir() / m_FileName;
            if (!std::filesystem::exists(path))
            {
                std::ofstream ofs(path);
                ofs.close();
            }
        }

    private:
        std::string m_FileName;
    };

    struct RmCMD : public CMD
    {
        RmCMD(CONSOLE_CMD cmd, RzServer* server, const std::string& path)
            : CMD(cmd, server)
            , m_path(path) {}

        virtual void Run() override
        {
            std::filesystem::path path = m_Server->GetCurrentDir() / m_path;
            if (std::filesystem::exists(path))
            {
                std::filesystem::remove_all(path);
            }
        }

    private:
        std::string m_path;
    };
}
