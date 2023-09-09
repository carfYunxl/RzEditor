#include "pch.h"
#include "RzCore/Log.hpp"
#include "RzServer/RzServer.hpp"
#include "RzCMD/TcpCmdParser.h"
#include "RzCMD/CMD.hpp"
#include "RzUtility/Utility.hpp"
#include "RzFile/FileTraveler.hpp"
#include "RzConsole/RzConsole.hpp"

namespace RzLib
{
	RzServer::RzServer(std::string&& ip, int port)
		: m_ip(std::move(ip))
		, m_port(std::move(port))
		, m_listen_socket(INVALID_SOCKET)
		, m_IsRunning(true)
		, m_Mode {InputMode::CONSOLE}
		, m_client_socket{ INVALID_SOCKET }
		, m_DirPath{ std::filesystem::current_path() }
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
	void RzServer::AcceptInput()
	{
		std::thread thread([&]()
		{
			ConsoleCMDParser parser(this);
			std::string sInput;

			while ( m_IsRunning )
			{
				Utility::PrintConsoleHeader(m_DirPath.string());
				getline(std::cin, sInput);
				if (sInput.empty()) continue;

				switch (m_Mode)
				{
					case InputMode::CONSOLE:
					{
						parser.SetCMD(sInput);
						parser.RunCmd();
						break;
					}
					case InputMode::SELECT:
					{
						//TO DO
						break;
					}
					case InputMode::SEND:
					{
						// 被设置成该模式，意味着接下来获取到的输入都应当发送给client
						// 或者是一条退出指令
						if (sInput == QUIT)
						{
							SetInputMode(InputMode::CONSOLE);
							continue;
						}

						// 发送给client
						SendInfo( TCP_CMD::NORMAL, sInput );
						break;
					}
				}
			}

			Log(LogLevel::WARN, "Server closed input!");
		});

		thread.detach();
	}

	// 处理客户端发过来的请求
	void RzServer::AcceptClient(SOCKET socket, const char* CMD, int rtLen)
	{
		TcpCmdParser parser(socket, CMD, rtLen);

		parser.RunCmd();
	}

	bool RzServer::Accept()
	{
		FD_ZERO(&m_All_FD);
		FD_SET(m_listen_socket, &m_All_FD);

		Log(LogLevel::INFO, "Server is listening ...\n");

		// 处理服务器的CMD
		AcceptInput();


		while ( m_IsRunning )
		{
			fd_set tmp = m_All_FD;
			int res = select(0, &tmp, NULL, NULL, 0);
			std::cout << std::endl;
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
						AcceptConnection();
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
						AcceptClient(tmp.fd_array[i], readBuf, len);
					}
				}
			}
		}

		Log(LogLevel::WARN, "Server stop accept client request!");

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

	void RzServer::AcceptConnection()
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

		m_client_socket = socket_client;

		// when client connected to server, we send client version to client
		std::string version;
		version.push_back(CLIENT_VERSION & 0xFF);
		version.push_back((CLIENT_VERSION >> 8) & 0xFF);
		SendInfo( TCP_CMD::VERSION, version );
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

	bool RzServer::IsClientSocket(size_t nSocket)
	{
		return m_client.end() != std::find_if(m_client.begin(), m_client.end(), [=](const std::pair<SOCKET,int>& pair)
		{
			return pair.first == static_cast<SOCKET>(nSocket);
		});
	}

	void RzServer::Start()
	{
		if ( !Init() )
		{
			RzLib::Log(RzLib::LogLevel::ERR, "server init error, error code : ", WSAGetLastError());
			return;
		}

		if ( !Listen() )
		{
			RzLib::Log(RzLib::LogLevel::ERR, "server listen error, error code : ", WSAGetLastError());
			return;
		}

		if ( !Accept() )
		{
			RzLib::Log(RzLib::LogLevel::ERR, "server accept error, error code : ", WSAGetLastError());
			return;
		}
	}

	void RzServer::SendInfo( TCP_CMD cmd, const std::string& msg )
	{
		std::string sSend;
		sSend.push_back(static_cast<char>(cmd));
		sSend.push_back( msg.size() & 0xFF );
		sSend.push_back( (msg.size() >> 8) & 0xFF );
		sSend += msg;

		if ( INVALID_SOCKET == send(m_client_socket, sSend.c_str(), static_cast<int>(sSend.size()), 0) )
		{
			Log(LogLevel::ERR, "send message to client error : ", WSAGetLastError());
		}
	}
}
