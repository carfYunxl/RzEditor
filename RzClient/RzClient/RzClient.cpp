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

namespace RzLib
{
    RzClient::RzClient(const std::string& server_ip, uint32_t server_port)
        : m_serverIp(server_ip)
        , m_serverPort(server_port)
        , m_socket(INVALID_SOCKET)
        , m_updated{false}
        , m_client_version{ 0x1001 }
    {
    }
    RzClient::RzClient(std::string&& server_ip, uint32_t&& server_port)
        : m_serverIp(std::move(server_ip))
        , m_serverPort(std::move(server_port))
        , m_socket(INVALID_SOCKET)
        , m_updated{ false }
        , m_client_version{ 0x1001 }
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
                Log(LogLevel::ERR, " erver refused connect request! ");
            }
            else
            {
                Log(LogLevel::ERR, "Connect error, error code : ", WSAGetLastError());
            }
            return false;
        }

        Log(LogLevel::INFO, "Connect to server success!");

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
        if (recv(m_socket, (char*)&sCh[0], size, 0) == SOCKET_ERROR)
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
                int msgSize = packet.second.size();
                printf("[CMD] : %02X\n", static_cast<int>(packet.first));
                printf("[SIZE] : %02X\n", static_cast<int>(msgSize));

                for (size_t i = 0; i < msgSize; ++i)
                {
                    if (i != 0 && i % 16 == 0)
                    {
                        printf("\n");
                    }
                    printf("%02X ", static_cast<unsigned char>(packet.second[i]));
                }

                switch (packet.first)
                {
                    case TCP_CMD::NORMAL:
                    {
                        Log(LogLevel::INFO, "server say : ", packet.second);
                        Utility::PrintConsoleHeader();
                        std::cout << std::endl;
                        break;
                    }
                    case TCP_CMD::VERSION:
                    {
                        std::cout << std::endl;
                        Log(LogLevel::INFO, "server send newest client version to me : ", packet.second);
                        size_t newVer = (packet.second[1] << 8) | packet.second[0];
                        m_client_version = newVer;
                        if (newVer != CLIENT_VERSION)
                        {
                            Log(LogLevel::WARN, "new Client now avaliable, update now ?[y/n]");
                            char chIn;
                            bool update = false;

                            std::cout << std::endl;

                            std::cin >> chIn;

                            if (chIn == 'y')
                            {
                                UpdateClient();
                            }
                        }
                        Utility::PrintConsoleHeader();
                        std::cout << std::endl;
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

                        Log(LogLevel::ERR, "server say : ", packet.second);

                        m_pCurPath = m_pRootPath / packet.second;

                        if (!m_pCurPath.has_extension())
                        {
                            //是目录
                            std::filesystem::create_directory(m_pCurPath);
                            Log(LogLevel::WARN, "接收到目录 ： ", m_pCurPath);
                        }
                        else
                        {
                            m_fCurContent.clear();
                            Log(LogLevel::WARN, "接收到文件 ： ", m_pCurPath);
                        }
                        Utility::PrintConsoleHeader();
                        std::cout << std::endl;
                        break;
                    }
                    case TCP_CMD::FILE_PACKET:
                    {
                        m_fCurContent.append(packet.second);
                        Log(LogLevel::WARN, "//================== 接收文件总大小 = ", m_fCurContent.size());

                        Log(LogLevel::WARN, packet.second.size());
                        Utility::PrintConsoleHeader();
                        std::cout << std::endl;
                        break;
                    }
                    case TCP_CMD::File_TAIL:
                    {
                        Log(LogLevel::WARN, "接收到文件包尾, 写入文件：");

                        std::ofstream out(m_pCurPath, std::ios::binary);
                        if (!out.is_open())
                        {
                            continue;
                        }

                        out.write(m_fCurContent.c_str(), m_fCurContent.size());
                        out.flush();
                        out.close();

                        m_fCurContent.clear();
                        Utility::PrintConsoleHeader();
                        std::cout << std::endl;

                        break;
                    }
                    case TCP_CMD::UPDATE_FIN:
                    {
                        // 更新客户端结束，将存储文件的目录恢复到Cache目录下
                        CreateDir("Cache");

                        // 关闭当前客户端， 启动更新程序，将Tmp目录下的文件搬到运行目录下
                        // 再重新启动客户端
                        std::filesystem::path path = std::filesystem::current_path() / "Update.exe";
                        Update(path);
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
            Utility::PrintConsoleHeader();

            std::cin.getline(&readBuf[0], 64);

            if (readBuf.empty())
            {
                continue;
            }

            if (memcmp(&readBuf[0],"exit",4) == 0)
            {
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

    bool RzClient::RecvFile(size_t fileSize, const std::string& filepath)
    {
        std::string strFileCache;
        std::string strRecv;
        strRecv.resize(MAX_TCP_PACKAGE_SIZE);
        while (fileSize > 0)
        {
            memset(&strRecv[0], 0, MAX_TCP_PACKAGE_SIZE);
            int ret = recv(m_socket, &strRecv[0], MAX_TCP_PACKAGE_SIZE, 0);

            if (ret == SOCKET_ERROR)
            {
                Log(LogLevel::ERR, "Recv file from server failed!, error code : ", WSAGetLastError());
                return false;
            }

            size_t len = strlen(&strRecv[0]);
            if (len != MAX_TCP_PACKAGE_SIZE)
                strFileCache += strRecv.substr(0, len);
            else
                strFileCache += strRecv;

            Log(LogLevel::INFO, "recv file success, total = ", strFileCache.size());

            fileSize -= MAX_TCP_PACKAGE_SIZE;
        }

        std::ofstream ofs(filepath, std::ios::out);
        if (ofs.is_open())
        {
            ofs.write(strFileCache.c_str(), strFileCache.size());
            ofs.flush();
            ofs.close();

            Log(LogLevel::INFO, "Recv file from server success!");
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

    //接收服务器发过来的可执行档
    bool RzClient::RecvExe(size_t fileSize, const std::string& filepath)
    {
        std::filesystem::path path = std::filesystem::current_path();
        path = path.parent_path();
        path /= "binClient";

        if (!std::filesystem::exists(path))
        {
            std::filesystem::create_directory(path);
        }

        Log(LogLevel::INFO, "bin path ", path.string());

        RecvFile(fileSize, filepath);

        return true;
    }

    void RzClient::CreateDir(const std::string& dirName)
    {
        m_pRootPath = std::filesystem::current_path() / dirName;

        if (!std::filesystem::exists(m_pRootPath))
        {
            std::filesystem::create_directory(m_pRootPath);
        }
    }

    void RzClient::Update(const std::filesystem::path& path)
    {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // Start the child process. 
        if (!CreateProcess(NULL,    // No module name (use command line)
            (LPSTR)(path.wstring().c_str()),    // Command line
            NULL,                   // Process handle not inheritable
            NULL,                   // Thread handle not inheritable
            FALSE,                  // Set handle inheritance to FALSE
            0,                      // No creation flags
            NULL,                   // Use parent's environment block
            NULL,                   // Use parent's starting directory 
            &si,                    // Pointer to STARTUPINFO structure
            &pi)                    // Pointer to PROCESS_INFORMATION structure
            )
        {
            Log(LogLevel::ERR, "CreateProcess failed, error code : ", GetLastError());
            return;
        }

        Log(LogLevel::INFO, "CreateProcess success!");

        // Wait until child process exits.
        //WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles. 
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        StopClient();
    }
}
