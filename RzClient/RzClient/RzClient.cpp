#include "RzClient.hpp"
#include "RzCore/Log.hpp"
#include "RzThread/RzThread.hpp"
#include "CMD/CMDParser.hpp"
#include "CMD/CMDType.hpp"
#include "CMD/TcpParser.h"
#include <fstream>
#include <memory>
#include "RzUtility/utility.hpp"
#include <windows.h>
#include "Extern/simpleini/SimpleIni.h"

namespace RzLib
{
    RzClient::RzClient(const std::string& server_ip, uint32_t server_port)
        : m_serverIp(server_ip)
        , m_serverPort(server_port)
        , m_socket(INVALID_SOCKET)
        , m_clientPath(std::filesystem::current_path())
    {

    }
    RzClient::RzClient(std::string&& server_ip, uint32_t&& server_port)
        : m_serverIp(std::move(server_ip))
        , m_serverPort(std::move(server_port))
        , m_socket(INVALID_SOCKET)
        , m_clientPath(std::filesystem::current_path())
    {
    }

    RzClient::~RzClient()
    {
        closesocket(m_socket);
        WSACleanup();
    }

    bool RzClient::Init()
    {
        WSADATA data;
        if ( WSAStartup(MAKEWORD(2, 2), &data) < 0 )
        {
            Log(LogLevel::ERR, "winsock2 start error !");
            return false;
        }

        m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (m_socket == INVALID_SOCKET)
        {
            Log(LogLevel::ERR, "create socket failed!");
            return false;
        }

        return true;

    }
    bool RzClient::Connect()
    {
        SOCKADDR_IN serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(m_serverPort);
        inet_pton(AF_INET, m_serverIp.c_str(), &serverAddr.sin_addr);

        if ( connect(m_socket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR )
        {
            int ret = WSAGetLastError();
            if (ret == WSAECONNREFUSED)
            {
                Log(LogLevel::ERR, " server refused connect request! ");
            }
            else
            {
                Log(LogLevel::ERR, " Connect error, error code : ", WSAGetLastError());
            }
            return false;
        }

        Log(LogLevel::INFO, "Connect to server success!");

        //Utility::PrintConsoleHeader(m_clientPath.string());

        Recv();
        Send();
        return true;
    }

    std::pair<TCP_CMD, std::string> RzClient::ReadPacket()
    {
        // 先读3个byte，确定CMD的类型，同时获取msg的长度
        std::vector<unsigned char> sCh(3);

        if ( recv(m_socket, (char*)&sCh[0], 3, 0) == SOCKET_ERROR )
        {
            Log(LogLevel::ERR, "Recv from server error, error code : ", WSAGetLastError());
            return {};
        }

        TCP_CMD cCmd = static_cast<TCP_CMD>(sCh[0]);

        size_t size = sCh[1] | (sCh[2] << 8);

        if (size == 0)
            return{ cCmd, ""};

        // 再以msg的长度来获取msg的资料
        sCh.clear();
        sCh.resize(size);
        if (recv(m_socket, (char*)&sCh[0], static_cast<int>(size), 0) == SOCKET_ERROR)
        {
            Log(LogLevel::ERR, "Recv from server error, error code : ", WSAGetLastError());
            return {};
        }

        std::string msg{sCh.begin(), sCh.end()};

        return { cCmd , msg };
    }

    // 这里收到的都是服务器传过来的数据
    bool RzClient::Recv()
    {
        ScopeThread sThread(std::thread([=]()
        {
            while (1)
            {
                auto packet = ReadPacket();
                switch (packet.first)
                {
                    case TCP_CMD::NORMAL:
                    {
                        std::cout << "\n";
                        Log(LogLevel::INFO, "server say : ", packet.second);
                        //Utility::PrintConsoleHeader();
                        std::cout << std::endl;
                        break;
                    }
                    case TCP_CMD::VERSION:
                    {
                        char str[8]{0};
                        sprintf_s(str, 8, "%02x%02x", packet.second[1], packet.second[0]);
                        std::string strVer(str);

                        if ( strVer != m_iniInfo.client_ver )
                        {
                            m_iniInfo.client_ver = strVer;

                            Log(LogLevel::INFO, "客户端发现新版本 : ", strVer, ", 要更新吗?[y/n]");
                            char chIn;
                            bool update = false;

                            std::cin >> chIn;

                            if (chIn == 'y')
                            {
                                UpdateClient();
                            }
                        }

                        //Utility::PrintConsoleHeader();
                        break;
                    }
                    case TCP_CMD::UPDATE_START:
                    {
                        //开始更新客户端了，在这里应该准备好存储文件的路径，因为此处与一般情况下不同
                        CreateDir("Tmp");
                        break;
                    }
                    case TCP_CMD::FILE_HEADER:
                    {
                        m_pCurPath.clear();

                        m_pCurPath = m_pRootPath / packet.second;

                        if (!m_pCurPath.has_extension())
                        {
                            //是目录
                            std::filesystem::create_directory(m_pCurPath);
                            Log(LogLevel::INFO, "接收到目录 ： ", m_pCurPath);
                        }
                        else
                        {
                            m_fCurContent.clear();
                            Log(LogLevel::INFO, "接收到文件 ： ", m_pCurPath);
                        }
                        //Utility::PrintConsoleHeader();
                        break;
                    }
                    case TCP_CMD::FILE_PACKET:
                    {
                        m_fCurContent.append(packet.second);
                        //Log(LogLevel::INFO, "//================== 接收文件总大小 = ", m_fCurContent.size());

                        //Log(LogLevel::WARN, packet.second.size());
                        break;
                    }
                    case TCP_CMD::File_TAIL:
                    {
                        Log(LogLevel::INFO, "接收到文件包尾, 写入文件：");

                        std::ofstream out(m_pCurPath, std::ios::binary);
                        if (!out.is_open())
                            continue;
                        out.write(m_fCurContent.c_str(), m_fCurContent.size());
                        out.flush();
                        out.close();

                        m_fCurContent.clear();
                        break;
                    }
                    case TCP_CMD::UPDATE_FIN:
                    {
                        // 关闭当前客户端， 启动更新程序，将Tmp目录下的文件搬到运行目录下
                        // 再重新启动客户端
                        std::filesystem::path path = std::filesystem::current_path() / "Update.exe";
                        Update(path);

                        Log(LogLevel::INFO, "客户端更新结束！");
                        //Utility::PrintConsoleHeader();
                        // 更新客户端结束，将存储文件的目录恢复到Cache目录下
                        CreateDir("Cache");
                        break;
                    }
                }
            }
        }));

        return true;
    }

    bool RzClient::Send()
    {
        std::string readBuf;
        readBuf.resize(64);

        while (m_Running)
        {
            std::cin.getline(&readBuf[0], 64);

            if (readBuf.empty())
            {
                continue;
            }

            if (memcmp(&readBuf[0],"exit",4) == 0)
            {
                StopClient();
                break;
            }

            size_t len = strlen(readBuf.c_str());

            readBuf.insert(readBuf.begin(), static_cast<char>(((len+3) >> 8) & 0xFF));
            readBuf.insert(readBuf.begin(), static_cast<char>((len+3) & 0xFF));
            readBuf.insert(readBuf.begin(), static_cast<char>(0xF1));

            if ( send(m_socket, &readBuf[0], static_cast<int>(len+3), 0) == SOCKET_ERROR)
            {
                Log(LogLevel::ERR, "send buffer to server failed, error code : ", WSAGetLastError());
                continue;
            }

            memset(&readBuf[0], 0, 64);
        }

        return true;
    }

    bool RzClient::UpdateClient()
    {
        // 客户端告诉服务器，我需要更新客户端，于是服务器开始准备将新的客户端文件发过来
        std::string strUpdate
        {
            static_cast<char>(0xF3),
            static_cast<char>(0x00),
            static_cast<char>(0x00)
        };
        if (SOCKET_ERROR == send(m_socket, strUpdate.c_str(), static_cast<int>(strUpdate.size()), 0))
        {
            return false;
        }
        return true;
    }

    void RzClient::CreateDir(const std::string& dirName)
    {
        m_pRootPath = std::filesystem::current_path().parent_path() / dirName;

        if (!std::filesystem::exists(m_pRootPath))
        {
            std::filesystem::create_directory(m_pRootPath);
        }
    }

    void RzClient::Update(const std::filesystem::path& path)
    {
        // 检查新版exe的版本后缀与服务器发过来的版本是否一致，不一致就删掉下载的内容
        // 遍历Tmp文件夹下的.exe档，并检查它们的版本

        for (auto const& dir_entry : std::filesystem::recursive_directory_iterator{ m_pRootPath })
        {
            std::string strExtent = dir_entry.path().extension().string();
            if ( dir_entry.path().has_extension() && (strExtent == ".exe" || strExtent == ".dll") )
            {
                std::string filestem = dir_entry.path().stem().string();

                std::string ver = filestem.substr(filestem.size() - 4, 4);

                Log(LogLevel::INFO, "client ver = ", ver);

                Log(LogLevel::INFO, "server client ver = ", m_iniInfo.client_ver);

                if (!ver.empty() && ver != m_iniInfo.client_ver)
                {
                    Log(LogLevel::ERR, "非法的版本程序，更新已暂停！");
                    //删除整个文件夹
                    std::filesystem::remove_all(m_pRootPath);
                    return;
                }
            }
        }
        
        // 重命名当前当前目录下的所有exe和dll
        for (auto const& dir_entry : std::filesystem::recursive_directory_iterator{ std::filesystem::current_path() })
        {
            std::filesystem::path path = dir_entry.path();
            std::string strExtent = path.extension().string();
            if (path.has_extension() && (strExtent == ".exe" || strExtent == ".dll"))
            {
                std::string stem = path.stem().string();
                stem.append("_old");
                std::filesystem::path path_after = (std::filesystem::current_path() / stem).string() + strExtent;

                std::filesystem::rename(path, path_after);
            }
        }

        // Tmp目录下的文件，拷贝到当前运行目录下
        for (auto const& dir_entry : std::filesystem::recursive_directory_iterator{ m_pRootPath })
        {
            std::filesystem::path curPath = dir_entry.path();
            std::string strExtent = curPath.extension().string();
            if ( !std::filesystem::is_directory(curPath) )
            {
                std::filesystem::path path = std::filesystem::current_path() / curPath.filename();

                if ( strExtent == ".exe" || strExtent == ".dll" )
                {
                    //直接拷贝
                    std::filesystem::copy_file(curPath, path);
                }
                else
                {
                    //删掉已有的，再拷贝
                    std::filesystem::remove(path);
                    std::filesystem::copy_file(curPath, path);
                }
            }      
        }
        std::filesystem::remove_all(m_pRootPath);
        
        // 启动新版程序
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        std::string sClient = "RzClient_" + m_iniInfo.client_ver + ".exe";

        std::filesystem::path clientPath = std::filesystem::current_path() / sClient;
        std::filesystem::path newClientPath = std::filesystem::current_path() / "RzClient.exe";

        std::filesystem::rename(clientPath, newClientPath);

        // Start the child process. 
        if (
            CreateProcess
            (
                NULL,
                (LPWSTR)(newClientPath.wstring().c_str()),  // No module name (use command line)                                                          // Command line
                NULL,                              // Process handle not inheritable
                NULL,                              // Thread handle not inheritable
                FALSE,                             // Set handle inheritance to FALSE
                0,                                 // No creation flags
                NULL,                              // Use parent's environment block
                NULL,                              // Use parent's starting directory 
                &si,                               // Pointer to STARTUPINFO structure
                &pi)                               // Pointer to PROCESS_INFORMATION structure
            )
        {
            Log(LogLevel::INFO, "CreateProcess success!");
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            StopClient();

            SaveIni();
            return;
        }
        Log(LogLevel::ERR, "CreateProcess failed, error code : ", GetLastError());     
    }

    // 加载ini的配置信息
    void RzClient::LoadIni()
    {
        std::filesystem::path ini_path = std::filesystem::current_path() / "Client.ini";

        CSimpleIniA ini;
        SI_Error rc = ini.LoadFile(ini_path.string().c_str());
        if (rc < 0) {
            RzLib::Log(RzLib::LogLevel::ERR, "open ini file failed : ", ini_path.string());
        };

        // if more ini section-key-value, update here
        const char* pv = ini.GetValue("Normal", "client_ver");

        m_iniInfo.client_ver = pv;
    }

    void RzClient::SaveIni()
    {
        std::filesystem::path ini_path = std::filesystem::current_path() / "Client.ini";

        CSimpleIniA ini;
        SI_Error rc = ini.LoadFile(ini_path.string().c_str());
        if (rc < 0) {
            RzLib::Log(RzLib::LogLevel::ERR, "open ini file failed : ", ini_path.string());
        };

        rc = ini.SetValue("Normal", "client_ver", m_iniInfo.client_ver.c_str());
        // if more ini section-key-value, update here
        if (rc < 0) {
            RzLib::Log(RzLib::LogLevel::ERR, "set ini value failed : ", ini_path.string());
        };
    }

    void RzClient::Run()
    {
        if ( !Init() )
        {
            RzLib::Log(RzLib::LogLevel::ERR, "client init error, error code : ", WSAGetLastError());
            return;
        }

        if ( !Connect() )
        {
            RzLib::Log(RzLib::LogLevel::ERR, "client connect error, error code : ", WSAGetLastError());
            return;
        }
    }
}
