#include "CMDParser.hpp"

namespace RzLib
{
	CMDParser::CMDParser(const std::string& CMD, char SPLIT)
	{
		Parser(CMD, SPLIT);
	}

	void CMDParser::SetCMD(const std::string& CMD, char SPLIT)
	{
		Parser(CMD, SPLIT);
	}

	void CMDParser::Parser(const std::string& CMD, char SPLIT)
	{
		if (CMD.empty())
		{
			m_cmdType = CMDType::NONE;
			return;
		}

		size_t index1 = CMD.find(SPLIT, 0);
		size_t index2 = CMD.find(SPLIT, index1 + 1);

		std::string strCmd = CMD.substr(0, index1);
		if (index1 == std::string::npos)
		{
			// 意味着这是一个单指令
			// e.g exit
			// e.g client
			strCmd.resize(strlen(&strCmd[0]));
			m_cmdType = CMDType::SINGLE;
			if (strCmd == "exit")
			{
				m_cmdInfo.CMD = static_cast<size_t>(ServerCMD::EXIT);
			}
			else if (strCmd == "client")
			{
				m_cmdInfo.CMD = static_cast<size_t>(ServerCMD::CLIENT);
			}
			return;
		}
		else if(index2 == std::string::npos)
		{
			// 这是一个俩指令
			// e.g 
			m_cmdType = CMDType::DOUBLE;
		}
		else
		{
			// 三指令
			// e.g send socket hello
			m_cmdType = CMDType::TRIPLE;
			if (strCmd == "file")
			{
				m_cmdInfo.CMD = static_cast<size_t>(ServerCMD::FILE);
			}
			else if (strCmd == "send")
			{
				m_cmdInfo.CMD = static_cast<size_t>(ServerCMD::SEND);
			}

			std::string sock = CMD.substr(index1 + 1, index2 - index1 - 1);

			if (IsAllDigits(sock))
			{
				m_cmdInfo.socket = static_cast<SOCKET>(stoi(sock));
			}
			m_cmdInfo.message = CMD.substr(index2 + 1, CMD.size() - index2 - 1);
		}
	}

	bool CMDParser::IsAllDigits(const std::string& digits)
	{
		return digits.cend() == std::find_if(digits.cbegin(), digits.cend(), [](const char ch){ return isalpha(ch);});
	}
}
