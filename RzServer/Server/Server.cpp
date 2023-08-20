#include "Server.hpp"
#include "Core/Log.hpp"
#include <thread>
#include <fstream>
#include "Core/Core.hpp"

#include <filesystem>
#include "CMD/Communication.h"

namespace RzLib
{
	Server::Server(std::string&& ip, int port)
		: m_ip(std::move(ip))
		, m_port(std::move(port))
		, m_listen_socket(INVALID_SOCKET)
		, m_IsRunning(true)
	{
	}
	Server::~Server()
	{
		closesocket(m_listen_socket);
		for (size_t i = 0; i < m_client.size(); ++i)
		{
			closesocket(m_client.at(i).first);
		}
		WSACleanup();
	}
	bool Server::Init()
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

	bool Server::Listen()
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
	void Server::HandleServerCMD()
	{
		std::thread thread([&]()
		{
				char readBuf[128]{0};
			while (1)
			{
				std::cin.getline( readBuf, 128 );

				if (strlen(readBuf) == 0) continue;

				ConsoleCMDParser parser(readBuf);

				std::unique_ptr<CMD> pCMD = GenCmd(parser);

				pCMD->Run();

				memset(readBuf,0,128);
			}
		});

		thread.detach();
	}

	// 处理客户端发过来的请求
	void Server::HandleClientCMD(SOCKET socket, const char* CMD, int rtLen)
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

				// 先告诉客户端，下面开始更新客户端的文件了
				const char* sendbuffer = "update";
				if ( SOCKET_ERROR == send(socket, sendbuffer, static_cast<int>(strlen(sendbuffer)), 0) )
				{
					Log(LogLevel::ERR,"send to client failed, error code = ",WSAGetLastError());
				}

				//遍历该目录
				std::string buffer;
				for (auto const& dir_entry : std::filesystem::directory_iterator{ binPath })
				{
					SendFileToClient( socket, "update", dir_entry.path().string());
				}
				break;
			}
			
		}
	}

	bool Server::Accept()
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

	void Server::ListClient()
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

	int Server::GetPort(SOCKET socket)
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

	void Server::AcceptClient()
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

	bool Server::GetClientMsg(SOCKET socket, char* buf, int* rtlen)
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

	bool Server::SendFileToClient(SOCKET socket, const std::string& Cmd, const std::string& path)
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

		std::string strCMD(Cmd + " " + std::to_string(size) + " " + path);

		if (send(socket, &strCMD[0], static_cast<int>(strCMD.size()), 0) == SOCKET_ERROR)
		{
			Log(LogLevel::ERR, "Send to client failed! \n");
			return false;
		}
		
		strCMD.clear();
		strCMD.resize(size);
		inf.seekg(0, std::ios::beg);
		inf.read(&strCMD[0], strCMD.size());
		inf.close();

		// 每次向client发送MAX_TCP_PACKAGE_SIZE数据，直到文件内容发送完毕
		size_t index = 0;
		Log(LogLevel::ERR, "All file size = ", size);
		while (int(size) > 0)
		{

			size_t sSize = size > MAX_TCP_PACKAGE_SIZE ? MAX_TCP_PACKAGE_SIZE : size;
			if (send(socket, &strCMD[index], static_cast<int>(sSize), 0) == SOCKET_ERROR)
			{
				Log(LogLevel::ERR, "Send to client failed! \n");
				return false;
			}

			index += sSize;
			size -= sSize;

			Log(LogLevel::INFO, "send file success, total = ", index);
		}

		Log(LogLevel::INFO, "Send file to client success! \n");

		return true;
	}

	// 发送最新的客户端版本给client
	bool Server::SendClientVersion(SOCKET socket)
	{
		std::string strVer;
		strVer.append(1,char(0xF2));								// 0xF2 : send new client version to client
		strVer.append(1,char(0x02));								// low byte
		strVer.append(1,char(0x00));								// high byte
		strVer.append(1,char(CLIENT_VERSION & 0xFF));				// version low byte
		strVer.append(1,char((CLIENT_VERSION >> 8) & 0xFF));		// version high byte

		if ( SOCKET_ERROR == send(socket, strVer.c_str(), static_cast<int>(strVer.size()), 0))
		{
			Log(LogLevel::ERR, "send to client failed! error code = ", WSAGetLastError());
			return false;
		}

		return true;
	}

	std::unique_ptr<CMD> Server::GenCmd(const ConsoleCMDParser& parser)
	{
		switch (parser.GetCmdType())
		{
		case CMDType::SINGLE:
			return std::make_unique<CMDSingle>(parser.GetCMD(),this);
		case CMDType::DOUBLE:
			return std::make_unique<CMDDouble>(parser.GetCMD(), this, parser.GetSocket());
		case CMDType::TRIPLE:
			return std::make_unique<CMDTriple>(parser.GetCMD(), this, parser.GetSocket(), parser.GetMsg());
		}

		return nullptr;
	}
}
