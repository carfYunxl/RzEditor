#include "ConsoleCMDParser.hpp"
#include "utility/utility.hpp"

#include <filesystem>

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

		std::string strCmd = CMD.substr(0, index1);

		if ( IsCmd(strCmd) )
		{
			if (index1 == std::string::npos)
			{
				// FUNC CMD
				m_cmdType = CMDType::FUNC;
				return;
			}
			else
			{
				// TRANSFER CMD
				m_cmdType = CMDType::TRANSFER;

				m_message = CMD.substr(index1 + 1, CMD.size() - index1 - 1);
			}
		}
		else
		{
			m_cmdType = CMDType::NONE;
			m_CMD = CONSOLE_CMD::UNKNOWN;
		}
	}

	const bool ConsoleCMDParser::IsCmd(const std::string& sCmd)
	{
		// e.g. 235£ºsend something to client socket : 235
		if ( sCmd.ends_with(':') && ServerUti::IsAllDigits(sCmd.substr(0,sCmd.size()-1)) )
		{
			m_CMD = CONSOLE_CMD::SEND;
			m_socket = stoi( sCmd.substr(0,sCmd.size()-1) );
			return true;
		}
		else if (sCmd == "exit" || sCmd == "client" || sCmd == "version")
		{
			m_CMD = CastCMD(sCmd);
			return true;
		}

		return false;
	}

	CONSOLE_CMD ConsoleCMDParser::CastCMD(const std::string& cmd)
	{
		if (cmd == "exit" )
		{
			return CONSOLE_CMD::EXIT;
		}
		else if (cmd =="client" )
		{
			return CONSOLE_CMD::CLIENT;
		}
		else if (cmd == "version")
		{
			return CONSOLE_CMD::VERSION;
		}

		return CONSOLE_CMD::UNKNOWN;
	}
}
