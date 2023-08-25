#include "Core/Log.hpp"
#include <filesystem>

#include "TcpServer/RzServer.hpp"
#include "CMD.hpp"

namespace RzLib
{
	CMD::CMD(const std::string& cmd, RzServer* server)
		: m_Cmd(cmd), m_Server(server)
	{
		//...
	}

	CMDSingle::CMDSingle(const std::string& cmd, RzServer* server)
		: CMD(cmd, server)
	{ }

    void CMDSingle::Run()
    {
		if (m_Cmd == "exit")
		{
			Log(LogLevel::WARN, "server is closed!");
		}
		else if (m_Cmd == "client")
		{
			m_Server->ListClient();
		}
		else if (m_Cmd == "path")
		{
			std::filesystem::path root = std::filesystem::current_path();
			Log(LogLevel::INFO, "Server exe path : ", root.string());
		}
		else {
			Log(LogLevel::ERR, "unknown single command!");
		}
    }

	CMDDouble::CMDDouble(const std::string& cmd, RzServer* server, SOCKET socket)
		: CMD(cmd, server), m_socket(socket)
	{}

    void CMDDouble::Run()
    {
		// TO DO
    }

	CMDTriple::CMDTriple(const std::string& cmd, RzServer* server, SOCKET socket, const std::string& msg)
		: CMD(cmd, server),m_socket(socket), m_message(msg)
	{

	}
    void CMDTriple::Run()
    {
		if (m_Cmd == "file")
		{
			m_Server->SendFileToClient(m_socket, m_message);
		}
		else if (m_Cmd == "send")
		{
			std::string strSend;
			strSend.append(1, static_cast<char>(0xF1));
			strSend.append(1, static_cast<char>(m_message.size() & 0xFF));
			strSend.append(1, static_cast<char>((m_message.size()>>8) & 0xFF));

			strSend += m_message;

			if (SOCKET_ERROR == send(m_socket, strSend.c_str(), static_cast<int>(strSend.size()), 0))
			{
				Log(LogLevel::ERR, "send to client error!");
			}
		}
		else if (m_Cmd == "exit")
		{
			m_Server->StopServer();
			Log(LogLevel::INFO, "server is closed!");
		}
		else
		{
			Log(LogLevel::ERR, "unknown triple command!");
		}
    }
}
