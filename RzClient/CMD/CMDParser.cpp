#include "CMDParser.hpp"
#include <string>

namespace RzLib
{
	//------------------------------------------------------------
	// CMDParser implementation									//
	//------------------------------------------------------------

	CMDParser::CMDParser(const std::string& CMD, char SPLIT)
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
		strCmd.resize(strlen(&strCmd[0]));
		if (index1 == std::string::npos)
		{
			// 意味着这是一个单指令
			// e.g exit
			// e.g client
			m_cmdType = CMDType::SINGLE;
			m_CMD = strCmd;
			return;
		}
		else if(index2 == std::string::npos)
		{
			// 这是一个俩指令
			// e.g 
			m_cmdType = CMDType::DOUBLE;
			m_CMD = strCmd;
			m_SecInfo = CMD.substr(index1 + 1, index2 - index1 - 1);
		}
		else
		{
			// 三指令
			// e.g send socket hello
			m_cmdType = CMDType::TRIPLE;
			m_CMD = strCmd;
			m_SecInfo = CMD.substr(index1 + 1, index2 - index1 - 1);
			m_message = CMD.substr(index2 + 1, CMD.size() - index2 - 1);
		}
	}

	bool CMDParser::IsAllDigits(const std::string& digits)
	{
		return digits.cend() == std::find_if(digits.cbegin(), digits.cend(), [](const char ch){ return isalpha(ch);});
	}
}
