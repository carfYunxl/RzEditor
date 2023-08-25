#include "Core/Log.hpp"
#include <ws2tcpip.h>
#include <thread>
#include <filesystem>
#include <fstream>
#include "TcpServer/RzServer.hpp"
#include "CMD/Communication.h"
#include "CMD/CMD.hpp"

namespace RzLib
{
	RzServer::RzServer(std::string&& ip, int port)
		: m_ip(std::move(ip))
		, m_port(std::move(port))
		, m_listen_socket(INVALID_SOCKET)
		, m_IsRunning(true)
	{
	}
	RzServer::~RzServer()
	{
		closesocket(m_listen_socket);
		for (size_t i = 0; i < m_client.size(); ++i)
		{
			closesocket(m_client.at(i).first);
		}
		WSACleanup();
	}
	bool RzServer::Init()
	{
		/*
		* socket start
		*/
		WSAData data;
		if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
		{
			Log(LogLevel::ERR, "Initialize win32 socket function failed!");
			return false;
		}

		return true;
	}

	bool RzServer::Listen()
	{
		/**
		* sockaddr_in
		*/
		sockaddr_in addr_server;
		addr_server.sin_family = AF_INET;//AF_INET6
		addr_server.sin_port = htons(m_port);
		inet_pton(AF_INET, m_ip.c_str(), &addr_server.sin_addr); //addr_server.sin_addr.s_addr = htonl(INADDR_ANY);

		/*
		* create server socket
		*/
		m_listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_listen_socket == INVALID_SOCKET)
		{

			Log(LogLevel::ERR, "create server socket failed, error code = ", WSAGetLastError());
			return false;
		}

		/*
		* bind socket
		*/
		if (bind(m_listen_socket, (sockaddr*)&addr_server, sizeof(sockaddr)) == SOCKET_ERROR)
		{
			Log(LogLevel::ERR, "socket bind error,error code = ", WSAGetLastError());
			return false;
		}

		/*
		* listen socket
		*/
		const int maxConnection = 5;
		if (listen(m_listen_socket, maxConnection) == SOCKET_ERROR)
		{
			Log(LogLevel::ERR, "listen socket error,error code = ", WSAGetLastError());
			return false;
		}

		return true;
	}

	// 响应服务端的相关CMD
	void RzServer::HandleServerCMD()
	{
		std::thread thread([&]()
		{
			ConsoleCMDParser parser;
			char readBuf[128]{0};
			while (1)
			{
				std::cin.getline( readBuf, 128 );

				if (strlen(readBuf) == 0) continue;

				parser.SetCMD(readBuf);

				std::unique_ptr<CMD> pCMD = GenCmd(&parser);

				pCMD->Run();

				memset(readBuf,0,128);
			}
		});

		thread.detach();
	}

	// 处理客户端发过来的请求
	void RzServer::HandleClientCMD(SOCKET socket, const char* CMD, int rtLen)
	{
		Communication parser(CMD,rtLen);
		switch (static_cast<CommunicationCMD>(parser.GetCmd()))
		{
			case CommunicationCMD::NORMAL:
			{
				Log(LogLevel::INFO, "Client ", socket, " Say:", parser.GetMsg(), "\n");
				break;
			}
			case CommunicationCMD::UPDATE:// 把编译好的exe发送给客户端
			{
				Log(LogLevel::INFO, "files in binClient : ");
				// 找到binClient的目录， 服务器需要在此处放置最新的客户端文件
				std::filesystem::path binPath = std::filesystem::current_path();
				binPath = binPath.parent_path();
				binPath /= "binClient";
				if (!std::filesystem::exists(binPath))
				{
					Log(LogLevel::ERR, "directory ",binPath, " not exist, please check!");
					return;
				}

				//遍历该目录
				std::string buffer;

				for (auto const& dir_entry : std::filesystem::recursive_directory_iterator{ binPath })
				{
					std::string path = dir_entry.path().filename().string();
					std::filesystem::path fPath = dir_entry.path().parent_path();
					while (fPath != binPath)
					{
						path.insert(0, fPath.filename().string() + "\\");
						fPath = fPath.parent_path();
					}
					//发送文件头
					buffer.push_back(0xF5);
					buffer.push_back(path.size() & 0xFF);
					buffer.push_back((path.size() >> 8) & 0xFF);

					//发送的是目录名或者文件名
					buffer += path;
					if ( send(socket, buffer.c_str(), buffer.size(), 0) == SOCKET_ERROR)
					{
						Log(LogLevel::ERR, "Send file name error!");
					}

					Log(LogLevel::WARN, "发送文件/名：",path);

					if (!dir_entry.path().has_extension())
					{
						continue;
					}

					// 发送文件
					SendFileToClient(socket, dir_entry.path().string());
				}
				break;
			}
			
		}
	}

	bool RzServer::Accept()
	{
		FD_ZERO(&m_All_FD);
		FD_SET(m_listen_socket, &m_All_FD);

		Log(LogLevel::INFO, "Server is listening ...\n");

		// 处理服务器的CMD
		HandleServerCMD();

		while (m_IsRunning)
		{
			fd_set tmp = m_All_FD;
			int res = select(0, &tmp, NULL, NULL, 0);
			if (res == SOCKET_ERROR)
			{
				Log(LogLevel::ERR, " select error, error code : ", WSAGetLastError());
				return false;
			}
			if (res == 0)
			{
				Log(LogLevel::INFO, "expired time limited ...");
				continue;
			}
			if (res > 0)
			{
				for (u_int i = 0; i < tmp.fd_count; ++i)
				{
					if (tmp.fd_array[i] == m_listen_socket)
					{
						// 处理客户端连接请求
						AcceptClient();
					}
					else
					{
						// 处理客户端消息
						char readBuf[MAX_TCP_PACKAGE_SIZE]{ 0 };
						int len = 0;
						if (!GetClientMsg(tmp.fd_array[i], readBuf, &len))
						{
							continue;
						}

						// 接收到了数据
						HandleClientCMD(tmp.fd_array[i], readBuf, len);
					}
				}
			}
		}

		return true;
	}

	void RzServer::ListClient()
	{
		if (m_client.empty())
		{
			Log(LogLevel::WARN, "No client is online...\n");
			return;
		}

		Log(LogLevel::INFO,"client:");
		std::cout << "\t=======" << std::endl;
		for (size_t i = 0;i < m_client.size();++i)
		{
			std::cout << "\t| " << i << ". " << m_client.at(i).first << std::endl;
		}

		std::cout << std::endl;
	}

	int RzServer::GetPort(SOCKET socket)
	{
		auto itr = std::find_if(m_client.begin(), m_client.end(), [=](const std::pair<SOCKET,int>& cli)
			{
				return cli.first == socket;
			});

		if (itr != m_client.end())
		{
			return (*itr).second;
		}

		return -1;
	}

	void RzServer::AcceptClient()
	{
		sockaddr_in clientaddr;
		int len = sizeof(sockaddr_in);
		SOCKET socket_client = accept(m_listen_socket, (sockaddr*)&clientaddr, &len);
		if (INVALID_SOCKET == socket_client)
		{
			return;
		}
		Log(LogLevel::INFO, "客户端已连接, socket : ", socket_client, " port : ", ntohs(clientaddr.sin_port), "\n");
		m_client.emplace_back(socket_client, ntohs(clientaddr.sin_port));

		FD_SET(socket_client, &m_All_FD);

		// when client connected to server, we send client version to client
		SendClientVersion(socket_client);
	}

	bool RzServer::GetClientMsg(SOCKET socket, char* buf, int* rtlen)
	{
		int res = recv(socket, buf, MAX_TCP_PACKAGE_SIZE, 0);
		if (res == 0)
		{
			// 连接被正常关闭
			Log(LogLevel::INFO, "Client ", socket, " offline...!\n");
			closesocket(socket);
			FD_CLR(socket, &m_All_FD);

			std::erase_if(m_client, [=](const std::pair<SOCKET, int>& cli)
				{
					return cli.first == socket;
				});
			return false;
		}
		else if (SOCKET_ERROR == res)
		{
			res = WSAGetLastError();
			if (res == WSAECONNRESET)
			{
				Log(LogLevel::INFO, "client ", socket, " offline...!\n");
				closesocket(socket);
				FD_CLR(socket, &m_All_FD);

				std::erase_if(m_client, [=](const std::pair<SOCKET, int>& cli)
					{
						return cli.first == socket;
					});
				return false;
			}
		}

		*rtlen = res;

		return true;
	}

	bool RzServer::SendFileToClient(SOCKET socket, const std::string& path)
	{
		// 先发给client，告诉client下面要发送的是文件

		std::ifstream inf(path);
		if (!inf.is_open())
		{
			Log(LogLevel::ERR, "Open file ", path , " failed!\n");
			return false;
		}

		inf.seekg(0, std::ios::end);
		size_t size = static_cast<size_t>(inf.tellg());
		if (size == -1)
		{
			Log(LogLevel::ERR, "Read file ", path, " failed!\n");
			return false;
		}
		
		std::string strFile;
		strFile.resize(size);
		inf.seekg(0, std::ios::beg);
		inf.read(&strFile[0], strFile.size());
		inf.close();

		// 每次向client发送MAX_TCP_PACKAGE_SIZE数据，直到文件内容发送完毕
		size_t index = 0;
		Log(LogLevel::ERR, "All file size = ", size);
		while (int(size) > 0)
		{
			size_t sSize = size > MAX_TCP_PACKAGE_SIZE - 3 ? MAX_TCP_PACKAGE_SIZE - 3 : size;

			std::string strSend;
			strSend.push_back(0xF6);
			strSend.push_back(sSize & 0xFF);
			strSend.push_back((sSize >> 8) & 0xFF);
			std::copy(&strFile[index], &strFile[index + size], std::back_inserter(strSend));

			if (send(socket, strSend.c_str(), strSend.size(), 0) == SOCKET_ERROR)
			{
				Log(LogLevel::ERR, "Send to client failed! \n");
				return false;
			}

			index += sSize;
			size -= sSize;

			Log(LogLevel::INFO, "send file success, total = ", index);
		}

		std::string strSend;
		strSend.push_back(0xF7);
		strSend.push_back(0x00);
		strSend.push_back(0x00);
		if (send(socket, strSend.c_str(), strSend.size(), 0) == SOCKET_ERROR)
		{
			Log(LogLevel::ERR, "Send to client failed! \n");
			return false;
		}

		Log(LogLevel::INFO, "Send file to client success! \n");

		return true;
	}

	// 发送最新的客户端版本给client
	bool RzServer::SendClientVersion(SOCKET socket)
	{
		std::string strVer;
		strVer.push_back(0xF2);
		strVer.push_back(0x02);
		strVer.push_back(0x00);
		strVer.push_back(CLIENT_VERSION & 0xFF);
		strVer.push_back((CLIENT_VERSION >> 8) & 0xFF);

		if ( SOCKET_ERROR == send(socket, strVer.c_str(), static_cast<int>(strVer.size()), 0))
		{
			Log(LogLevel::ERR, "send to client failed! error code = ", WSAGetLastError());
			return false;
		}

		return true;
	}

	std::unique_ptr<CMD> RzServer::GenCmd(const ConsoleCMDParser* parser)
	{
		switch (parser->GetCmdType())
		{
			case CMDType::SINGLE:
				return std::make_unique<CMDSingle>(parser->GetCMD(), this);
			case CMDType::DOUBLE:
				return std::make_unique<CMDDouble>(parser->GetCMD(), this, parser->GetSocket());
			case CMDType::TRIPLE:
				return std::make_unique<CMDTriple>(parser->GetCMD(), this, parser->GetSocket(), parser->GetMsg());
		}

		return nullptr;
	}
}
