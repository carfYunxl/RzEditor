#include "CMDParser.hpp"
#include <string>

namespace RzLib
{

	//------------------------------------------------------------
	// CMDParserBase implementation								//
	//------------------------------------------------------------
	CMDParserClient::CMDParserClient(const std::string& CMD, char SPLIT)
		: m_CMD(0)
	{
		Parser(CMD, SPLIT);
	}

	void CMDParserClient::Parser(const std::string& CMD, char SPLIT)
	{
		if (CMD.empty())
		{
			return;
		}

		std::string strCmd = CMD;

		strCmd.resize(strlen(&strCmd[0]));
		if (strCmd == "port")
		{
			m_CMD = static_cast<size_t>(ClientCMD::PORT);
		}
		else if (strCmd == "ip")
		{
			m_CMD = static_cast<size_t>(ClientCMD::IP);
		}
		else if (strCmd == "update")
		{
			m_CMD = static_cast<size_t>(ClientCMD::UPDATE);
		}
		else
		{
			m_CMD = static_cast<size_t>(ClientCMD::NORMAL);
		}
		return;
	}

	void CMDParserClient::SetCMD(const std::string& CMD, char SPLIT)
	{
		this->Parser(CMD,SPLIT);
	}
	//------------------------------------------------------------
	// CMDParser implementation									//
	//------------------------------------------------------------

	CMDParser::CMDParser(const std::string& CMD, char SPLIT)
		: CMDParserClient(CMD,SPLIT)
	{
		Parser(CMD, SPLIT);
	}

	void CMDParser::SetCMD(const std::string& CMD, char SPLIT)
	{
		this->Parser(CMD, SPLIT);
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
				m_CMD = static_cast<size_t>(ServerCMD::EXIT);
			}
			else if (strCmd == "client")
			{
				m_CMD = static_cast<size_t>(ServerCMD::CLIENT);
			}
			else if (strCmd == "path")
			{
				m_CMD = static_cast<size_t>(ServerCMD::PATH);
			}
			else
			{
				m_CMD = 4;
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
				m_CMD = static_cast<size_t>(ServerCMD::FILE);
			}
			else if (strCmd == "send")
			{
				m_CMD = static_cast<size_t>(ServerCMD::SEND);
			}
			else
			{
				m_CMD = 4;
			}

			std::string sock = CMD.substr(index1 + 1, index2 - index1 - 1);

			if (IsAllDigits(sock))
			{
				m_socket = static_cast<SOCKET>(stoi(sock));
			}
			m_message = CMD.substr(index2 + 1, CMD.size() - index2 - 1);
		}
	}

	bool CMDParser::IsAllDigits(const std::string& digits)
	{
		return digits.cend() == std::find_if(digits.cbegin(), digits.cend(), [](const char ch){ return isalpha(ch);});
	}
}
