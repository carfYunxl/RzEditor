#include "CMDType.hpp"
#include "Core/Core.hpp"
#include "Core/Log.hpp"
#include <filesystem>

namespace RzLib
{
    void CMDSingle::Run()
    {
		ServerCMD	CMD = static_cast<ServerCMD>(m_CmdId);
		switch (CMD)
		{
		case ServerCMD::EXIT:
			Log(LogLevel::WARN, "server is closed!");
			break;
		case ServerCMD::CLIENT:
			m_Server->ListClient();
			break;
		case ServerCMD::PATH:
		{
			std::filesystem::path root = std::filesystem::current_path();
			Log(LogLevel::INFO, "Server exe path : ", root.string());
			break;
		}
		case ServerCMD::UNKNOWN:
			Log(LogLevel::ERR, "unknown single command!");
			break;
		}
    }

    void CMDDouble::Run()
    {
		ServerCMD	CMD = static_cast<ServerCMD>(m_CmdId);
		//SOCKET		socket = parser.GetSocket();
		// ...

		switch (CMD)
		{
		default:
			Log(LogLevel::ERR, "No second command definition now!");
		}
    }

    void CMDTriple::Run()
    {
		ServerCMD	CMD = static_cast<ServerCMD>(m_CmdId);

		switch (CMD)
		{
			case ServerCMD::FILE:
				m_Server->SendFileToClient(m_socket, m_message);
				break;
			case ServerCMD::SEND:
				if (SOCKET_ERROR == send(m_socket, m_message.c_str(), static_cast<int>(m_message.size()), 0))
				{
					Log(LogLevel::ERR, "send to client error!");
				}
				break;
			case ServerCMD::UNKNOWN:
				Log(LogLevel::ERR, "unknown triple command!");
				break;
		}
    }
}
