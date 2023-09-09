#include "TcpCmdParser.h"
#include "RzCore/Core.hpp"
#include "RzServer/RzServer.hpp"
#include "RzCore/Log.hpp"
#include <filesystem>
#include "RzFile/FileTraveler.hpp"

namespace RzLib
{
	TcpCmdParser::TcpCmdParser(SOCKET socket, const char* bufCmd, int rtLen)
		: m_socket(socket)
		, m_Cmd(0)
		, m_msg("")
    {
		Parser(bufCmd, rtLen);
	}

	void TcpCmdParser::Parser(const char* cmd, int rtLen)
	{
		// byte[0]是cmd类型
		m_Cmd = cmd[0];

		if (!IsCmd())
		{
			m_Cmd = 0;
			return;
		}

		unsigned short size = cmd[2] << 8 | cmd[1];

		if (size != 0)
		{
			m_msg.resize(size);
			std::copy(&cmd[3], &cmd[size], m_msg.begin());
		}
	}

	const bool TcpCmdParser::IsCmd() const
	{
		return g_ClientCMD.end() != std::find( g_ClientCMD.begin(), g_ClientCMD.end(), m_Cmd );
	}

	void TcpCmdParser::RunCmd()
	{
		switch (m_Cmd)
		{
			case 0xF1:
				Log(LogLevel::INFO, "Client ", m_socket, " Say:", m_msg, "\n");
				break;

			case 0xF3:
			{
				Log(LogLevel::INFO, "files in binClient : ");
				// 找到binClient的目录， 服务器需要在此处放置最新的客户端文件
				std::filesystem::path binPath = std::filesystem::current_path();
				binPath /= "binClient";
				if (!std::filesystem::exists(binPath))
				{
					->Log(LogLevel::ERR, "directory ", binPath, " not exist, please check!");
					return;
				}

				//遍历该目录
				std::string buffer{
					static_cast<char>(0xF4),
					static_cast<char>(0x00),
					static_cast<char>(0x00)
				};
				if (send(m_socket, buffer.c_str(), static_cast<int>(buffer.size()), 0) == SOCKET_ERROR)
				{
					m_UI->Log(LogLevel::ERR, "Send update start error!");
				}

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
					buffer = {
					static_cast<char>(0xF5),
					static_cast<char>(path.size() & 0xFF),
					static_cast<char>((path.size() >> 8) & 0xFF)
					};

					//发送的是目录名或者文件名
					buffer += path;
					if (send(m_socket, buffer.c_str(), static_cast<int>(buffer.size()), 0) == SOCKET_ERROR)
					{
						Log(LogLevel::ERR, "Send file name error!");
					}

					Log(LogLevel::WARN, "发送文件/名：", path);

					if (dir_entry.path().has_extension())
					{
						// 发送文件
						SendFileToClient(dir_entry.path().string());
					}
				}

				//发送更新结束的标志给客户端
				buffer = {
					static_cast<char>(0xF8),
					static_cast<char>(0x00),
					static_cast<char>(0x00)
				};
				std::cout << "Finish CMD : " << buffer.c_str() << std::endl;
				std::cout << "buffer size = " << buffer.size() << std::endl;
				if (SOCKET_ERROR == send(m_socket, buffer.c_str(), static_cast<int>(buffer.size()), 0))
				{
					Log(LogLevel::ERR, "Send file end error!");
				}
				break;
			}

			default:
				break;
		}
	}

	bool TcpCmdParser::SendFileToClient(const std::string& path)
	{
		FileTraveler file(path);
		if (!file.open(Mode::Binary))
		{
			Log(LogLevel::ERR, "open file : ", path, " failed!");
			return false;
		}
		std::string strFile = file.GetFileContent();
		file.close();

		size_t size = strFile.size();

		// 每次向client发送MAX_TCP_PACKAGE_SIZE数据，直到文件内容发送完毕
		size_t index = 0;
		Log(LogLevel::ERR, "All file size = ", size);

		std::string strSend;

		while (int(size) > 0)
		{
			size_t sSize = size > MAX_TCP_PACKAGE_SIZE - 3 ? MAX_TCP_PACKAGE_SIZE - 3 : size;

			strSend = {
				static_cast<char>(0xF6),
				static_cast<char>(sSize & 0xFF),
				static_cast<char>((sSize >> 8) & 0xFF),
			};

			strSend.resize(sSize + 3);

			memcpy(&strSend[3], &strFile[index], sSize);

			if (send(m_socket, strSend.c_str(), static_cast<int>(strSend.size()), 0) == SOCKET_ERROR)
			{
				Log(LogLevel::ERR, "Send to client failed! \n");
				return false;
			}

			index += sSize;
			size -= sSize;

			//Log(LogLevel::INFO, "send file success, total = ", index);
			strSend.clear();
		}

		strSend = {
				static_cast<char>(0xF7),
				static_cast<char>(0x00),
				static_cast<char>(0x00),
		};

		if (send(m_socket, strSend.c_str(), static_cast<int>(strSend.size()), 0) == SOCKET_ERROR)
		{
			Log(LogLevel::ERR, "Send to client failed! \n");
			return false;
		}

		Log(LogLevel::INFO, "Send file to client success! \n");

		return true;
	}
}
