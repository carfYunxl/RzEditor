#include "pch.h"
#include "RzServer/RzServer.hpp"
#include "RzCMD/TcpCmdParser.h"
#include "RzCMD/CMD.hpp"
#include "RzUtility/Utility.hpp"
#include "RzFile/FileTraveler.hpp"
#include "RzConsole/RzConsole.hpp"
#include "RzThread/RzThread.hpp"

namespace RzLib
{
	RzServer::RzServer(RzSlim* UI, std::string&& ip, int port)
		: m_UI(UI)
		, m_ip(std::move(ip))
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
		int ret = WSAStartup(MAKEWORD(2, 2), &data);
		int res = WSAGetLastError();

		if (ret!= 0)
		{
			m_UI->Log_NextLine(LogLevel::ERR, "Initialize win32 socket function failed!");
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
			m_UI->Log_NextLine( LogLevel::ERR, QString("create server socket failed, error code = %1").arg(WSAGetLastError()) );
			return false;
		}

		/*
		* bind socket
		*/
		if (bind(m_listen_socket, (sockaddr*)&addr_server, sizeof(sockaddr)) == SOCKET_ERROR)
		{
			m_UI->Log_NextLine( LogLevel::ERR, QString("socket bind error,error code = %1").arg(WSAGetLastError()) );
			return false;
		}

		/*
		* listen socket
		*/
		const int maxConnection = 5;
		if (listen(m_listen_socket, maxConnection) == SOCKET_ERROR)
		{
			m_UI->Log_NextLine( LogLevel::ERR, QString("listen socket error,error code = %1").arg(WSAGetLastError()) );
			return false;
		}

		return true;
	}

	// 处理客户端发过来的请求
	void RzServer::AcceptClient(SOCKET socket, const char* CMD, int rtLen)
	{
		TcpCmdParser parser(this, socket, CMD, rtLen);

		parser.RunCmd();
	}

	void RzServer::AcceptRequest()
	{
		while ( m_IsRunning )
		{
			fd_set tmp = m_All_FD;
			int res = select(0, &tmp, NULL, NULL, 0);
			if (res == SOCKET_ERROR)
			{
				//m_UI->Log_NextLine(LogLevel::ERR, QString(" select error, error code : %1").arg(WSAGetLastError()));
				return;
			}
			if (res == 0)
			{
				m_UI->Log_NextLine(LogLevel::INFO, "expired time limited ... ");
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
		m_UI->Log_NextLine(LogLevel::WARN, "Server stop accept client request!");
	}

	bool RzServer::Accept()
	{
		FD_ZERO(&m_All_FD);
		FD_SET(m_listen_socket, &m_All_FD);

		m_UI->GetStateLabel()->setText("Server is listening ...");
		PrintConsoleHeader(m_DirPath.string());

		// 处理服务器的CMD
		std::thread th_request(std::bind(&RzServer::AcceptRequest, this));
		th_request.detach();
		return true;
	}

	void RzServer::ListClient()
	{
		if (m_client.empty())
		{
			m_UI->Log_NextLine(LogLevel::WARN, "No client is online..." );
			return;
		}

		m_UI->Log_NextLine(LogLevel::INFO, "client:" );
		m_UI->Log_NextLine(LogLevel::INFO, "\t=======" );
		for (size_t i = 0;i < m_client.size();++i)
		{
			m_UI->Log_NextLine(LogLevel::INFO, QString("\t| %1. %2").arg(i).arg(m_client.at(i).first) );
		}
		m_UI->Log_NextLine( LogLevel::INFO, " " );
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
		m_UI->GetStateLabel()->setText(QString("Client connected, socket : %1 port : %2").arg(socket_client).arg(ntohs(clientaddr.sin_port)));
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
			m_UI->Log_NextLine( LogLevel::INFO, QString("Client %1 offline...!").arg(socket) );
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
				m_UI->Log_NextLine( LogLevel::INFO, QString("client %1 offline...!").arg(socket) );
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
			m_UI->Log_NextLine( RzLib::LogLevel::ERR, QString( "server init error, error code : %1").arg(WSAGetLastError()) );
			return;
		}

		if ( !Listen() )
		{
			m_UI->Log_NextLine( RzLib::LogLevel::ERR, QString( "server listen error, error code : %1").arg(WSAGetLastError()) );
			return;
		}

		if ( !Accept() )
		{
			m_UI->Log_NextLine( RzLib::LogLevel::ERR, QString( "server accept error, error code : %1").arg(WSAGetLastError()) );
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
			m_UI->Log_NextLine(LogLevel::ERR, QString("send message to client error : %1").arg(WSAGetLastError()) );
		}
	}

	void RzServer::PrintConsoleHeader(const std::string& path)
	{
		m_UI->Log_NextLine(LogLevel::NORMAL,"");
		m_UI->Log_ThisLine(LogLevel::CONSOLE, QString("%1 ").arg(Utility::GetUserInfo().c_str()));
		m_UI->Log_ThisLine(LogLevel::WARN, QString("%1 ").arg(path.c_str()));
		m_UI->Log_NextLine(LogLevel::INFO, QString("$ "));
		m_UI->Log_ThisLine(LogLevel::NORMAL, "");
	}
}
