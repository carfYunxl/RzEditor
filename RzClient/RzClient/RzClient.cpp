#include "RzClient.hpp"
#include "Core/Log.hpp"
#include "RzThread/RzThread.hpp"
#include "CMD/CMDParser.hpp"
#include "CMD/CMDType.hpp"
#include "CMD/TcpParser.h"
#include <fstream>
#include <memory>
#include "cUtility/cUtility.hpp"

namespace RzLib
{
    RzClient::RzClient(const std::string& server_ip, uint32_t server_port)
        : m_serverIp(server_ip)
        , m_serverPort(server_port)
        , m_socket(INVALID_SOCKET)
        , m_updated{false}
    {
    }
    RzClient::RzClient(std::string&& server_ip, uint32_t&& server_port)
        : m_serverIp(std::move(server_ip))
        , m_serverPort(std::move(server_port))
        , m_socket(INVALID_SOCKET)
        , m_updated{ false }
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

    // 这里收到的都是服务器传过来的数据
    bool RzClient::Recv()
    {
        ScopeThread sThread(std::thread([=]()
        {
            char readBuf[MAX_TCP_PACKAGE_SIZE]{0};
            while (1)
            {
                int ret = recv(m_socket, readBuf, MAX_TCP_PACKAGE_SIZE, 0);
                std::cout << std::endl;
                if (ret == SOCKET_ERROR)
                {
                    int ret = WSAGetLastError();
                    if (ret == WSAECONNABORTED)
                    {
                        Log(LogLevel::ERR, " Disconnect from server! ");
                    }
                    else if (ret == WSAECONNRESET)
                    {
                        Log(LogLevel::ERR, " server closed, connected aborted! ");
                    }
                    else
                    {
                        Log(LogLevel::ERR, "Recv from server error, error code : ",ret );
                    }
                    return false;
                }
                else
                {
                    TcpParser parser(readBuf, ret);

                    std::vector<RecvInfo> info = parser.GetInfo();
                    std::string msg;

                    for (size_t i = 0;i < info.size(); ++i)
                    {
                        RECV_CMD cCmd = static_cast<RECV_CMD>(info.at(i).cmd);
                        msg = info.at(i).msg;

                        switch (cCmd)
                        {
                            case RECV_CMD::NORMAL:
                            {
                                Log(LogLevel::INFO, "server say : ", msg);
                                ClientUti::PrintConsoleHeader();
                                std::cout << std::endl;
                                break;
                            }
                            case RECV_CMD::VERSION:
                            {
                                Log(LogLevel::INFO, "server send newest client version to me : ", msg);
                                size_t newVer = (msg[0] << 8) | msg[1];
                                if (newVer != CLIENT_VERSION)
                                {
                                    UpdateClient();
                                }
                                ClientUti::PrintConsoleHeader();
                                std::cout << std::endl;
                                break;
                            }
                            case RECV_CMD::UPDATE_START:
                            {
                                //开始更新客户端了，在这里应该准备好存储文件的路径，因为此处与一般情况下不同
                                CreateDir("Tmp");
                                break;
                            }
                            case RECV_CMD::FILE_HEADER:
                            {
                                m_pCurPath.clear();

                                Log(LogLevel::ERR, "server say : ", msg);

                                memset(readBuf, 0, MAX_TCP_PACKAGE_SIZE);

                                m_pCurPath = m_pRootPath / msg;

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
                                ClientUti::PrintConsoleHeader();
                                std::cout << std::endl;
                                break;
                            }
                            case RECV_CMD::FILE_PACKET:
                            {
                                m_fCurContent += msg;

                                Log(LogLevel::WARN, "接收到文件包：", "现在的文件内容是：", m_fCurContent);
                                ClientUti::PrintConsoleHeader();
                                std::cout << std::endl;
                                break;
                            }
                            case RECV_CMD::File_TAIL:
                            {
                                Log(LogLevel::WARN, "接收到文件包尾, 写入文件：", m_pCurPath);
                                std::ofstream out(m_pCurPath, std::ios::out);
                                if (out.is_open())
                                {
                                    out.write(m_fCurContent.c_str(), m_fCurContent.size());
                                    out.flush();
                                    out.close();
                                }

                                m_fCurContent.clear();
                                ClientUti::PrintConsoleHeader();
                                std::cout << std::endl;
                                break;
                            }
                            case RECV_CMD::UPDATE_FIN:
                            {
                                //更新客户端结束，将存储文件的目录恢复到Cache目录下
                                CreateDir("Cache");
                                Update("");
                                break;
                            }
                        }
                    }
                    memset(readBuf, 0, MAX_TCP_PACKAGE_SIZE);
                }
            }
        }));

        return true;
    }

    bool RzClient::Send()
    {
        std::string readBuf;
        readBuf.resize(64);

        while (1)
        {
            ClientUti::PrintConsoleHeader();

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

            readBuf.insert(readBuf.begin(), static_cast<char>((len >> 8) & 0xFF));
            readBuf.insert(readBuf.begin(), static_cast<char>(len & 0xFF));
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
        std::string strUpdate;
        strUpdate.append(1, static_cast<char>(0xF3));
        strUpdate.append(1, static_cast<char>(0x00));
        strUpdate.append(1, static_cast<char>(0x00));
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

    void RzClient::Update(const std::string& path)
    {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // Start the child process. 
        if (!CreateProcess(NULL,    // No module name (use command line)
            (LPSTR)path.c_str(),    // Command line
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
            printf("CreateProcess failed (%d).\n", GetLastError());
            return;
        }

        // Wait until child process exits.
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles. 
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}
