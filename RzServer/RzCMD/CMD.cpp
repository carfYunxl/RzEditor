#include "RzCore/Log.hpp"
#include <filesystem>
#include "RzServer/RzServer.hpp"
#include "CMD.hpp"
#include "RzUtility/utility.hpp"

namespace RzLib
{
	CMD::CMD(CONSOLE_CMD cmd, RzServer* server)
		: m_Cmd(cmd), m_Server(server)
	{
		//...
	}

	FuncCMD::FuncCMD(CONSOLE_CMD cmd, RzServer* server)
		: CMD(cmd, server)
	{ }

    void FuncCMD::Run()
    {
		switch (m_Cmd)
		{
			case CONSOLE_CMD::CLIENT:
			{
				m_Server->ListClient();
				break;
			}
			case CONSOLE_CMD::VERSION:
			{
				break;
			}
			case CONSOLE_CMD::EXIT:
			{
				m_Server->StopServer();
				Log(LogLevel::INFO, "server is closed!");
				break; 
			}
			case CONSOLE_CMD::UNKNOWN:
			{
				Log(LogLevel::ERR, "unknown command!");
				break;
			}
		}
    }

	TransferCMD::TransferCMD(CONSOLE_CMD cmd, RzServer* server, SOCKET socket, const std::string& msg)
		: CMD(cmd, server),m_socket(socket), m_message(msg)
	{

	}
    void TransferCMD::Run()
    {
		switch (m_Cmd)
		{
			case CONSOLE_CMD::SEND:
			{
				if (std::filesystem::exists(m_message))
				{
					// 查看输入是否是一个路径
					m_Server->SendFileToClient(m_socket, m_message);
				}
				else
				{
					// 不是路径就是一条信息
					size_t size = m_message.size();
					std::string strSend{
						static_cast<char>(0xF1),
						static_cast<char>(size & 0xFF),
						static_cast<char>((size >> 8) & 0xFF)
					};
					strSend += m_message;

					if (SOCKET_ERROR == send(m_socket, strSend.c_str(), static_cast<int>(strSend.size()), 0))
					{
						Log(LogLevel::ERR, "send info to client error!");
					}
				}
				break;
			}
			case CONSOLE_CMD::CLIENT:
			{
				m_Server->ListClient();
				break;
			}
			case CONSOLE_CMD::VERSION:
			{
				// ...
				break;
			}
			case CONSOLE_CMD::EXIT:
			{
				m_Server->StopServer();
				Log(LogLevel::INFO, "server is closed!");
				break;
			}
			case CONSOLE_CMD::UNKNOWN:
			{
				Log(LogLevel::ERR, "unknown single command!");
				break;
			}
		}
    }
}
