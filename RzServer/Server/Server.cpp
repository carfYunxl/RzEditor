#include "Server.hpp"
#include "Log.hpp"

namespace RzLib
{
	Server::Server(std::string&& ip, int port)
		: m_ip(std::move(ip))
		, m_port(std::move(port))
	{
	}
	Server::~Server()
	{
		closesocket(m_listen_socket);
		for (size_t i = 0; i < client.size(); ++i)
		{
			closesocket(client.at(i).first);
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

	bool Server::Accept()
	{
		FD_ZERO(&m_All_FD);
		FD_SET(m_listen_socket, &m_All_FD);

		while (1)
		{
			fd_set tmp = m_All_FD;
			int res = select(0, &tmp, NULL, NULL, 0);
			if (res == SOCKET_ERROR)
			{
				Log(LogLevel::ERR, "Socket error : ", WSAGetLastError());
				return false;
			}
			else if (res == 0)
			{
				Log(LogLevel::INFO, "expired time limited ...");
				continue;
			}
			else if (res > 0)
			{
				for (u_int i = 0; i < tmp.fd_count; ++i)
				{
					if (tmp.fd_array[i] == m_listen_socket)
					{
						//accept
						sockaddr_in clientaddr;
						int len = sizeof(sockaddr_in);
						SOCKET socket_client = accept(m_listen_socket, (sockaddr*)&clientaddr, &len);
						if (INVALID_SOCKET == socket_client)
						{
							continue;
						}
						Log(LogLevel::INFO, "客户端已连接, socket : ", socket_client, " port : ", ntohs(clientaddr.sin_port));
						client.emplace_back(socket_client, ntohs(clientaddr.sin_port));

						FD_SET(socket_client, &m_All_FD);
						if (SOCKET_ERROR == send(socket_client, "成功连接到服务器！", 10, 0))
						{
							Log(LogLevel::ERR, " error occured,error code : ", WSAGetLastError());
							return false;
						}
					}
					else
					{
						//处理客户端消息
						char readBuf[1500]{ 0 };
						int nRecv = recv(tmp.fd_array[i], readBuf, 1500, 0);
						if (nRecv == 0)
						{
							//连接被正常关闭
							Log(LogLevel::WARN, "客户端 ：", tmp.fd_array[i], " 已断开连接！ ");
							closesocket(tmp.fd_array[i]);
							FD_CLR(tmp.fd_array[i], &m_All_FD);
						}
						else if (SOCKET_ERROR == nRecv)
						{
							//出错
							Log(LogLevel::ERR, "recv error : error code : ", WSAGetLastError());
							return false;
						}
						//接收到了数据
						Log(LogLevel::INFO, "Read from client : ", tmp.fd_array[i], " success! ");
						Log(LogLevel::INFO, readBuf);
					}
				}
			}
		}
	}
}
