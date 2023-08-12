#include "Server.hpp"
#include "Core/Log.hpp"
#include <thread>
#include <fstream>
#include "Core/Core.hpp"
#include "CMD/CMDParser.hpp"

namespace RzLib
{
	Server::Server(std::string&& ip, int port)
		: m_ip(std::move(ip))
		, m_port(std::move(port))
		, m_listen_socket(INVALID_SOCKET)
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
			std::string strSend;
			strSend.resize(128);

			CMDParser parser("");
			while (1)
			{
				std::cin.getline(&strSend[0], 128);

				if (strSend.empty())
				{
					continue;
				}

				parser.SetCMD(strSend);

				switch (parser.GetCmdType())
				{
				case CMDType::NONE:
					continue;
					break;
				case CMDType::SINGLE:
				{
					ServerCMD	CMD = static_cast<ServerCMD>(parser.GetCMD());
					switch (CMD)
					{
					case ServerCMD::EXIT:
						Log(LogLevel::WARN, "server is closed!");
						break;
					case ServerCMD::CLIENT:
						ListClient();
						break;
					default:
						Log(LogLevel::ERR, "unknown single command!");
						break;
					}
				}
					break;
				case CMDType::DOUBLE:
				{
					ServerCMD	CMD = static_cast<ServerCMD>(parser.GetCMD());
					SOCKET		socket = parser.GetSocket();
					// ...

					switch (CMD)
					{
					default:
						Log(LogLevel::ERR, "No second command definition now!");
					}
				}
					break;
				case CMDType::TRIPLE:
				{
					ServerCMD	CMD = static_cast<ServerCMD>(parser.GetCMD());
					SOCKET		socket = parser.GetSocket();
					std::string msg = parser.GetMsg();

					switch (CMD)
					{
					case ServerCMD::FILE:
						SendFileToClient(socket, msg);
						break;
					case ServerCMD::SEND:
						if (SOCKET_ERROR == send(socket, msg.c_str(), static_cast<int>(msg.size()), 0))
						{
							Log(LogLevel::ERR, "send to client error!");
						}
						break;
					default:
						Log(LogLevel::ERR, "unknown triple command!");
						break;
					}
				}
					break;
				}

				memset(&strSend[0],0,strSend.size());
			}
		});

		thread.detach();
	}

	// 处理客户端发过来的请求
	void Server::HandleClientCMD(SOCKET socket, const char* CMD)
	{
		if (strcmp(CMD,"port") == 0)
		{
			std::string strPort = std::to_string(m_port);
			send(socket, &strPort[0], static_cast<int>(strPort.size()), 0);
		}
		else if (strcmp(CMD, "ip") == 0)
		{
			send(socket, &m_ip[0], static_cast<int>(m_ip.size()), 0);
		}
		else
		{
			Log(LogLevel::INFO, "Client ", socket, " Say:", CMD, "\n");
		}
	}

	bool Server::Accept()
	{
		FD_ZERO(&m_All_FD);
		FD_SET(m_listen_socket, &m_All_FD);

		Log(LogLevel::INFO, "Server is listening ...\n");

		// 处理服务器的CMD
		HandleServerCMD();

		while (1)
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
						char readBuf[1500]{ 0 };
						if (!GetClientMsg(tmp.fd_array[i], readBuf))
						{
							continue;
						}

						// 接收到了数据
						HandleClientCMD(tmp.fd_array[i], readBuf);
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
		for (size_t i = 0;i < m_client.size();++i)
		{
			std::cout << m_client.at(i).first << std::endl;
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

		const char* info = "成功连接到服务器！";
		if (SOCKET_ERROR == send(socket_client, info, static_cast<int>(strlen(info)), 0))
		{
			Log(LogLevel::ERR, " error occured,error code : ", WSAGetLastError());
		}
	}

	bool Server::GetClientMsg(SOCKET socket, char* buf)
	{
		int res = recv(socket, buf, 1500, 0);
		if (res == 0)
		{
			//连接被正常关闭
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

		return true;
	}

	bool Server::SendFileToClient(SOCKET socket, const std::string& path)
	{
		//先发给client，告诉client下面要发送的是文件

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

		std::string strCMD("file " + std::to_string(size) + " " + path);

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

		// 每次向client发送1K数据，知道文件内容发送完毕
		int index = 0;
		while (int(size) > 0)
		{
			size_t sSize = size > 1024 ? 1024 : size;
			if (send(socket, &strCMD[index], static_cast<int>(sSize), 0) == SOCKET_ERROR)
			{
				Log(LogLevel::ERR, "Send to client failed! \n");
				return false;
			}

			index += 1024;
			size -= 1024;
		}

		Log(LogLevel::INFO, "Send file to client success! \n");

		return true;
	}
}
