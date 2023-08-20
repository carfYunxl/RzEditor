#include "CMDType.hpp"
#include "Core/Core.hpp"
#include "Core/Log.hpp"
#include <filesystem>


namespace RzLib
{
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

    void CMDDouble::Run()
    {
		// TO DO
    }

    void CMDTriple::Run()
    {
		if (m_Cmd == "file")
		{
			m_Server->SendFileToClient(m_socket, "file", m_message);
		}
		else if (m_Cmd == "send")
		{
			std::string strSend;
			strSend.append(1,0xF1);
			strSend.append(1, m_message.size() & 0xFF);
			strSend.append(1, (m_message.size()>>8) & 0xFF);

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
