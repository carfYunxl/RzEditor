/*****************************************************************//**
 * \file   ConsoleCMDParser.cpp
 * \brief  ConsoleCMDParser class have two mainly functions:
 * 
 *	1 - tells the cmd type : trans cmd or func cmd?
 * 
 *		what is trans cmd ? 
 *			> just like : socket: msg...
 *			> 234: hello client ...
 *			> 234: file path or others ...
 *			> ...etc
 * 
 *		what is func cmd ? just like:
 *			> exit
 *			> client
 *			> ls
 *			> ...etc
 * 
 *		how to recognize this two type cmd?
 *			> our cmd parser will find whether this input cmd line have space, no means only one word input, this should be a func cmd.
 *				then we will check whether its valid
 * 
 *			> otherwise, we think its should be a trans cmd, so check valid again
 * 
 *	2 - collect important information
 *		
 *		for func cmd:
 *			> cmd type: exit / client / ls / ...
 * 
 *		for trans cmd:
 *			> socket
 *			> message or filepath or link ...etc
 * 
 * \author yun
 * \date   September 2023
 *********************************************************************/

#include "pch.h"
#include "RzCMD/CMD.hpp"
#include "ConsoleCMDParser.hpp"
#include "RzUtility/Utility.hpp"
#include "RzServer/RzServer.hpp"

namespace RzLib
{
	//------------------------------------------------------------
	// CMDParser implementation									//
	//------------------------------------------------------------

	ConsoleCMDParser::ConsoleCMDParser(const std::string& CMD, char SPLIT)
		: m_CMD(CONSOLE_CMD::UNKNOWN)
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
		if ( IsTransCMD(sCmd) )
		{
			m_CMD = CONSOLE_CMD::SEND;
			m_socket = stoi( sCmd.substr(0,sCmd.size()-1) );
			return true;
		}
		else if ( IsFunCMD(sCmd) )
		{
			m_CMD = CastCMD(sCmd);
			return true;
		}
		else
		{
			m_CMD = CONSOLE_CMD::UNKNOWN;
		}

		return false;
	}


	// New CMD add here
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

	const bool ConsoleCMDParser::IsFunCMD(const std::string& fCMD)
	{
		return g_FunCMD.end() != std::find( g_FunCMD.begin(), g_FunCMD.end(), fCMD);
	}

	const bool ConsoleCMDParser::IsTransCMD(const std::string& tCMD)
	{
		return tCMD.ends_with(':') && Utility::IsAllDigits(tCMD.substr(0, tCMD.size() - 1));
	}

	void ConsoleCMDParser::RunCmd(RzServer* server)
	{
		std::unique_ptr<CMD> pCmd;
		switch (m_CMD)
		{
		case CONSOLE_CMD::SEND:
			pCmd = std::make_unique<SendCMD>(m_CMD, server, m_socket, m_message);
			break;
		case CONSOLE_CMD::EXIT:
			pCmd = std::make_unique<ExitCMD>(m_CMD, server);
			break;
		case CONSOLE_CMD::CLIENT:
			pCmd = std::make_unique<ClientCMD>(m_CMD, server);
			break;
		case CONSOLE_CMD::VERSION:
			pCmd = std::make_unique<VersionCMD>(m_CMD, server);
			break;
		case CONSOLE_CMD::UNKNOWN:
			pCmd = nullptr;
			break;
		}
		
		if (pCmd)
			pCmd->Run();
		else
			Log(LogLevel::ERR, "unknown command!\n");
	}
}
