#include "ConsoleCMDParser.hpp"

namespace RzLib
{
	//------------------------------------------------------------
	// CMDParser implementation									//
	//------------------------------------------------------------

	ConsoleCMDParser::ConsoleCMDParser(const std::string& CMD, char SPLIT)
	{
		Parser(CMD, SPLIT);
	}

	void ConsoleCMDParser::SetCMD(const std::string& CMD, char SPLIT)
	{
		this->Parser(CMD, SPLIT);
	}

	void ConsoleCMDParser::Parser(const std::string& CMD, char SPLIT)
	{
		size_t index1 = CMD.find(SPLIT, 0);
		size_t index2 = CMD.find(SPLIT, index1 + 1);

		std::string strCmd = CMD.substr(0, index1);
		if (index1 == std::string::npos)
		{
			// Single CMD
			m_cmdType = CMDType::SINGLE;
			m_CMD = strCmd;
			return;
		}
		else if(index2 == std::string::npos)
		{
			// Double CMD
			// TO BE DEFINE
			m_cmdType = CMDType::DOUBLE;
		}
		else
		{
			// Triple CMD
			m_cmdType = CMDType::TRIPLE;
			m_CMD = strCmd;

			std::string sock = CMD.substr(index1 + 1, index2 - index1 - 1);

			if (IsAllDigits(sock))
			{
				m_socket = static_cast<SOCKET>(stoi(sock));
			}
			m_message = CMD.substr(index2 + 1, CMD.size() - index2 - 1);
		}
	}

	const bool ConsoleCMDParser::IsAllDigits(const std::string& digits) const
	{
		return digits.cend() == std::find_if(digits.cbegin(), digits.cend(), [](const char ch){ return isalpha(ch);});
	}
}
